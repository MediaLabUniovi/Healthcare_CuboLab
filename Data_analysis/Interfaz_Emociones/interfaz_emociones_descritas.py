import streamlit as st
import pandas as pd
import requests
from io import StringIO
import plotly.graph_objects as go
from statsmodels.tsa.statespace.sarimax import SARIMAX

# URLs de descarga directa desde OSF
urls = {
    "emociones.csv": "https://osf.io/download/6746f7b034e6f1b711ac1e5f/",
}

# Función para cargar los datos sin cache
def cargar_datos(url, skip_lines=0):
    response = requests.get(url)
    response.raise_for_status()
    data_str = response.text
    return pd.read_csv(StringIO(data_str), skiprows=skip_lines)

# Inicializar el estado de los datos al inicio
if "emociones" not in st.session_state:
    st.session_state.emociones = cargar_datos(urls["emociones.csv"])

# Botón para actualizar manualmente la base de datos
if st.button("Actualizar Base de Datos"):
    st.session_state.emociones = cargar_datos(urls["emociones.csv"])
    st.success("Datos actualizados desde el servidor OSF")

# Cargar el dataframe desde el estado
df = st.session_state["emociones"]

# Asegurarse de que la columna de fecha esté en formato datetime
if not df.empty and "fecha" in df.columns:
    df["fecha"] = pd.to_datetime(df["fecha"], errors="coerce")

# Título de la aplicación
st.title("Análisis y Predicción de Emociones por Usuario con Descripción")

# Selección de usuario
usuarios_disponibles = df["id_paciente"].unique()
id_usuario = st.selectbox("Seleccione un usuario:", usuarios_disponibles)

# Filtrar datos para el usuario seleccionado
datos_usuario = df[df["id_paciente"] == id_usuario]

if datos_usuario.empty:
    st.warning("No hay datos disponibles para este usuario.")
else:
    # Ordenar los datos por fecha
    datos_usuario = datos_usuario.sort_values(by="fecha")

    # Crear gráfica escalonada para datos históricos
    fig = go.Figure()
    fig.add_trace(
        go.Scatter(
            x=datos_usuario["fecha"],
            y=datos_usuario["emocion"],
            mode="lines+markers",
            line_shape="hv",
            name=f"Usuario {id_usuario}",
            hovertext=datos_usuario["intervencion"],  # Mostrar intervención en el hover
            hoverinfo="text+x+y"  # Mostrar intervención, fecha y emoción
        )
    )

    # Configuración de diseño
    fig.update_layout(
        title=f"Gráfica Escalonada de Emociones con Descripciones para el Usuario {id_usuario}",
        xaxis_title="Fecha y Hora",
        yaxis_title="Emoción",
        template="plotly_white",
        xaxis=dict(tickformat="%Y-%m-%d %H:%M", showgrid=True),
        yaxis=dict(showgrid=True)
    )

    # Mostrar gráfica histórica
    st.plotly_chart(fig)

    # Selección de cantidad de días para predecir
    st.write("### Configuración de Predicción")
    dias_a_predecir = st.slider("Seleccione el número de días para predecir:", min_value=1, max_value=30, value=10)

    # Predicción
    st.write(f"### Predicción para los próximos {dias_a_predecir} días")
    emociones_diarias = datos_usuario.groupby(datos_usuario["fecha"].dt.date)["emocion"].mean()
    emociones_diarias.index = pd.to_datetime(emociones_diarias.index)

    # Entrenar modelo SARIMAX
    modelo = SARIMAX(emociones_diarias, order=(1, 1, 1), seasonal_order=(0, 1, 1, 7))
    modelo_ajustado = modelo.fit(disp=False)

    # Predicciones futuras
    prediccion = modelo_ajustado.get_forecast(steps=dias_a_predecir)
    fechas_prediccion = pd.date_range(start=emociones_diarias.index[-1] + pd.Timedelta(days=1), periods=dias_a_predecir, freq="D")
    predicciones = pd.DataFrame({
        "fecha": fechas_prediccion,
        "emocion": prediccion.predicted_mean.round().clip(1, 6).astype(int)
    })

    # Añadir predicción como gráfica escalonada independiente
    fig.add_trace(
        go.Scatter(
            x=predicciones["fecha"],
            y=predicciones["emocion"],
            mode="lines+markers",
            line_shape="hv",
            name=f"Predicción (Próximos {dias_a_predecir} días)",
            line=dict(dash="dot", color="red"),
            hovertext=["Predicción"] * dias_a_predecir,  # Texto fijo para predicciones
            hoverinfo="text+x+y"  # Mostrar fecha y emoción
        )
    )

    # Actualizar gráfica con predicción escalonada
    st.plotly_chart(fig)

    # Mostrar tabla de predicciones
    st.write(f"Tabla de Predicciones para los Próximos {dias_a_predecir} Días")
    st.dataframe(predicciones)
