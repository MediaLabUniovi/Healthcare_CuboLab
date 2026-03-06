import argparse
from pathlib import Path
import warnings
import re
from typing import Optional, Tuple

import numpy as np
import pandas as pd

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.dates as mdates


def _norm_col(x: str) -> str:
    return str(x).replace("\ufeff", "").strip().lower()


def _find_col(df: pd.DataFrame, name: str) -> str:
    name_norm = _norm_col(name)
    for c in df.columns:
        if _norm_col(c) == name_norm:
            return c
    raise KeyError(f"No encuentro la columna '{name}'. Columnas: {list(df.columns)}")


def _find_optional_col(df: pd.DataFrame, name: str) -> Optional[str]:
    try:
        return _find_col(df, name)
    except KeyError:
        return None


def _safe_name(x) -> str:
    s = str(x).strip()
    if s == "" or s.lower() == "nan":
        s = "sin_id"
    s = re.sub(r"[^a-zA-Z0-9._-]+", "_", s)
    return s[:80]


def _detect_time_col(df: pd.DataFrame) -> str:
    for cand in ["fecha", "hora", "timestamp", "datetime", "date", "time"]:
        c = _find_optional_col(df, cand)
        if c is not None:
            return c
    raise KeyError(f"No encuentro columna temporal. Columnas: {list(df.columns)}")


def build_datos_procesados_from_df(df_in: pd.DataFrame, col_time: str, col_emo: str) -> pd.DataFrame:
    """
    datos_procesados:
      - fecha (hora exacta)
      - emocion (0..5)
    Regla: último valor cronológico por hora + reindex horario completo + ffill/bfill.
    """
    df = df_in[[col_time, col_emo]].copy()
    df.rename(columns={col_time: "fecha_raw", col_emo: "emocion_raw"}, inplace=True)

    df["ts"] = pd.to_datetime(df["fecha_raw"], errors="coerce")
    df["emocion"] = pd.to_numeric(df["emocion_raw"], errors="coerce")
    df = df.dropna(subset=["ts", "emocion"]).copy()

    df.loc[~df["emocion"].between(0, 5), "emocion"] = np.nan
    df = df.dropna(subset=["emocion"]).copy()

    if df.empty:
        raise ValueError("No quedan datos válidos tras limpiar (fecha/emocion).")

    df = df.sort_values("ts")
    df["fecha_h"] = df["ts"].dt.floor("h")

    # último valor real dentro de cada hora (cronológico)
    df_last = df.groupby("fecha_h", as_index=False).tail(1)[["fecha_h", "emocion"]]
    df_last = df_last.sort_values("fecha_h").reset_index(drop=True)

    start, end = df_last["fecha_h"].iloc[0], df_last["fecha_h"].iloc[-1]
    full_idx = pd.date_range(start=start, end=end, freq="h")

    s = df_last.set_index("fecha_h")["emocion"].reindex(full_idx).ffill().bfill()
    s = np.clip(np.rint(s).astype(int), 0, 5)

    return pd.DataFrame({"fecha": full_idx, "emocion": s})


def load_hourly_series_from_procesados(df_procesados: pd.DataFrame) -> pd.DataFrame:
    df = df_procesados.copy()
    df.rename(columns={"fecha": "hora_dt"}, inplace=True)
    return df.sort_values("hora_dt").reset_index(drop=True)[["hora_dt", "emocion"]]

# Markov
def train_markov(states: np.ndarray, laplace: float = 1.0) -> np.ndarray:
    counts = np.zeros((6, 6), dtype=float)
    for a, b in zip(states[:-1], states[1:]):
        if 0 <= a <= 5 and 0 <= b <= 5:
            counts[int(a), int(b)] += 1.0
    counts += laplace
    return counts / counts.sum(axis=1, keepdims=True)


def predict_markov_one_step(train_states: np.ndarray) -> int:
    train_states = np.asarray(train_states, dtype=int)
    if len(train_states) == 0:
        return 0
    if len(train_states) == 1:
        return int(train_states[-1])
    P = train_markov(train_states, laplace=1.0)
    return int(np.argmax(P[int(train_states[-1])]))


def forecast_markov(states: np.ndarray, horizon: int) -> np.ndarray:
    states = np.asarray(states, dtype=int)
    if horizon <= 0:
        return np.array([], dtype=int)
    if len(states) == 0:
        return np.full(horizon, 0, dtype=int)
    if len(states) == 1:
        return np.full(horizon, int(states[-1]), dtype=int)

    P = train_markov(states, laplace=1.0)
    dist = np.zeros(6, dtype=float)
    dist[int(states[-1])] = 1.0

    preds = []
    for _ in range(horizon):
        dist = dist @ P
        nxt = int(np.argmax(dist))
        preds.append(nxt)
        dist = np.zeros(6, dtype=float)
        dist[nxt] = 1.0
    return np.array(preds, dtype=int)

# ETS
def _ets_forecast_numeric(y: np.ndarray, horizon: int) -> np.ndarray:
    y = np.asarray(y, dtype=float)
    if horizon <= 0:
        return np.array([], dtype=float)
    if len(y) == 0:
        return np.full(horizon, 0.0, dtype=float)
    if len(y) < 3:
        return np.full(horizon, float(y[-1]), dtype=float)

    try:
        from statsmodels.tsa.holtwinters import ExponentialSmoothing
        model = ExponentialSmoothing(y, trend="add", seasonal=None, initialization_method="estimated")
        fit = model.fit(optimized=True)
        return np.asarray(fit.forecast(horizon), dtype=float)
    except Exception as e:
        warnings.warn(f"ETS falló ({e}). Uso SES.")
        alpha = 0.3
        level = float(y[0])
        for val in y[1:]:
            level = alpha * float(val) + (1 - alpha) * level
        return np.full(horizon, level, dtype=float)


def forecast_ets_discrete(states: np.ndarray, horizon: int) -> np.ndarray:
    fc = _ets_forecast_numeric(states, horizon)
    return np.clip(np.rint(fc).astype(int), 0, 5)


def predict_ets_one_step(train_states: np.ndarray) -> int:
    fc = _ets_forecast_numeric(train_states, 1)
    return int(np.clip(int(np.rint(fc[0])) if len(fc) else 0, 0, 5))

# Walk-forward
def walk_forward_scores(states: np.ndarray, min_train: int = 24) -> dict:
    states = np.asarray(states, dtype=int)
    n = len(states)
    if n < max(5, min_train + 2):
        return {"n_test": 0, "markov_mae": np.inf, "ets_mae": np.inf, "markov_acc": 0.0, "ets_acc": 0.0}

    err_m, err_e, hit_m, hit_e = [], [], [], []
    for t in range(min_train, n - 1):
        train = states[: t + 1]
        y_true = int(states[t + 1])

        pm = predict_markov_one_step(train)
        pe = predict_ets_one_step(train)

        err_m.append(abs(pm - y_true))
        err_e.append(abs(pe - y_true))
        hit_m.append(1 if pm == y_true else 0)
        hit_e.append(1 if pe == y_true else 0)

    return {
        "n_test": len(err_m),
        "markov_mae": float(np.mean(err_m)) if err_m else np.inf,
        "ets_mae": float(np.mean(err_e)) if err_e else np.inf,
        "markov_acc": float(np.mean(hit_m)) if hit_m else 0.0,
        "ets_acc": float(np.mean(hit_e)) if hit_e else 0.0,
    }


def choose_model(scores: dict) -> str:
    if scores["n_test"] <= 0 or not (np.isfinite(scores["markov_mae"]) and np.isfinite(scores["ets_mae"])):
        return "markov"
    if scores["markov_mae"] < scores["ets_mae"] - 1e-9:
        return "markov"
    if scores["ets_mae"] < scores["markov_mae"] - 1e-9:
        return "ets"
    return "markov" if scores["markov_acc"] >= scores["ets_acc"] else "ets"

# Plot 
def save_plot_observed_plus_forecast(df_hist: pd.DataFrame,
                                     future_idx: pd.DatetimeIndex,
                                     fc_best: np.ndarray,
                                     out_path: Path) -> None:
    hist = df_hist.sort_values("hora_dt").copy()

    fc_best = np.asarray(fc_best, dtype=int)
    if len(future_idx) != len(fc_best):
        raise ValueError("future_idx y fc_best deben tener la misma longitud")

    plt.figure(figsize=(14, 5))
    plt.plot(hist["hora_dt"], hist["emocion"].astype(int),
             label="observado", drawstyle="steps-post", linewidth=1.8)
    plt.plot(future_idx, fc_best,
             label="predicción", drawstyle="steps-post", linewidth=2.2)

    plt.ylim(-0.5, 5.5)
    plt.yticks([0, 1, 2, 3, 4, 5])
    plt.xlabel("fecha")
    plt.ylabel("emocion")
    plt.title(f"Predicción (próximas {len(fc_best)} horas)")

    ax = plt.gca()
    locator = mdates.AutoDateLocator(minticks=6, maxticks=12)
    ax.xaxis.set_major_locator(locator)
    ax.xaxis.set_major_formatter(mdates.ConciseDateFormatter(locator))

    plt.legend()
    plt.tight_layout()
    plt.savefig(out_path, dpi=160)
    plt.close()


def main():
    parser = argparse.ArgumentParser(description="Predicción discreta (Markov vs ETS) por horas (0..5).")
    parser.add_argument("--csv", required=True)
    parser.add_argument("--horizon", type=int, default=48)
    parser.add_argument("--outdir", required=True)
    args = parser.parse_args()

    csv_path = Path(args.csv).expanduser()
    outdir = Path(args.outdir).expanduser()
    outdir.mkdir(parents=True, exist_ok=True)

    horizon = int(args.horizon)
    if horizon <= 0:
        raise ValueError("--horizon debe ser > 0")

    df_raw = pd.read_csv(csv_path, sep=None, engine="python", encoding="utf-8-sig")

    col_time = _detect_time_col(df_raw)
    col_emo = _find_col(df_raw, "emocion")
    col_pid = _find_optional_col(df_raw, "id_paciente")

    if col_pid is None:
        groups: Tuple[Tuple[str, pd.DataFrame], ...] = (("unico", df_raw.copy()),)
    else:
        # groupby robusto (evita líos de tipos)
        groups = tuple((pid, g.copy()) for pid, g in df_raw.groupby(col_pid, dropna=False))

    for pid, df_p in groups:
        pid_name = _safe_name(pid)

        if df_p.empty:
            continue

        # 1) datos_procesados por paciente (y ES lo que se usa después)
        df_procesados = build_datos_procesados_from_df(df_p, col_time=col_time, col_emo=col_emo)
        out_proc = outdir / f"datos_procesados_paciente_{pid_name}.csv"
        df_procesados.to_csv(out_proc, index=False)

        # 2) pipeline usa SOLO df_procesados
        df = load_hourly_series_from_procesados(df_procesados)
        y = df["emocion"].astype(int).to_numpy()

        min_train = min(24, max(5, len(y) // 3))
        scores = walk_forward_scores(y, min_train=min_train)
        chosen = choose_model(scores)

        fc_markov = forecast_markov(y, horizon=horizon)
        fc_ets = forecast_ets_discrete(y, horizon=horizon)
        fc_best = fc_markov if chosen == "markov" else fc_ets

        last_time = df["hora_dt"].iloc[-1]
        future_idx = pd.date_range(start=last_time + pd.Timedelta(hours=1), periods=horizon, freq="h")

        out_pred = outdir / f"prediccion_{horizon}h_paciente_{pid_name}.csv"
        pd.DataFrame({"fecha": future_idx, "emocion": fc_best.astype(int)}).to_csv(out_pred, index=False)

        out_plot = outdir / f"plot_prediccion_{horizon}h_paciente_{pid_name}.png"
        save_plot_observed_plus_forecast(df, future_idx, fc_best, out_plot)

        # impresión mínima para verificar
        print(f"[OK] paciente={pid_name} modelo={chosen} -> {out_proc.name}, {out_pred.name}, {out_plot.name}")


if __name__ == "__main__":
    main()
