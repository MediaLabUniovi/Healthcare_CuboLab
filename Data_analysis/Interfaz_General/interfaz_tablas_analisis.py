import streamlit as st
import pandas as pd
import os
from pathlib import Path
import altair as alt
from statsmodels.tsa.holtwinters import ExponentialSmoothing
from statsmodels.tsa.arima.model import ARIMA

# Configuración de Streamlit para mostrar todas las columnas
st.set_page_config(layout="wide")
pd.set_option('display.max_columns', None)
pd.set_option('display.max_colwidth', None)
pd.set_option('display.max_rows', None)

# Rutas de archivos locales
BASE_DIR = Path(__file__).parent
DATA_DIR = BASE_DIR / "data"

# Diccionario de rutas de archivos locales
archivos = {
    "enfermeras_1k.csv": DATA_DIR / "enfermeras.csv",
    "pacientes_enfermeras_new.csv": DATA_DIR / "pacientes_enfermeras.csv",
    "centros_100.csv": DATA_DIR / "centros.csv",
    "intervenciones_1k.csv": DATA_DIR / "intervenciones.csv",
    "medidas_1k.csv": DATA_DIR / "medidas.csv",
    "pacientes_1k.csv": DATA_DIR / "pacientes.csv",
    "registro_login_1k.csv": DATA_DIR / "registros_login.csv",
    "sensores_500.csv": DATA_DIR / "sensores.csv",
    "sesiones_1k.csv": DATA_DIR / "sesiones.csv",
    "tipo_sensor.csv": DATA_DIR / "tipos_sensor.csv"
}

# Función para cargar los datos sin cache
def cargar_datos(ruta_archivo, skip_lines):
    if not os.path.exists(ruta_archivo):
        st.error(f"Archivo no encontrado: {ruta_archivo}")
        return None
    return pd.read_csv(ruta_archivo, skiprows=skip_lines)

# Inicializar el estado de los datos al inicio
if "enfermeras" not in st.session_state:
    st.session_state.enfermeras = cargar_datos(archivos["enfermeras_1k.csv"], skip_lines=0)
if "pacientes_enfermeras" not in st.session_state:
    st.session_state.pacientes_enfermeras = cargar_datos(archivos["pacientes_enfermeras_new.csv"], skip_lines=0)
if "centros" not in st.session_state:
    st.session_state.centros = cargar_datos(archivos["centros_100.csv"], skip_lines=0)
if "intervenciones" not in st.session_state:
    st.session_state.intervenciones = cargar_datos(archivos["intervenciones_1k.csv"], skip_lines=0)
if "medidas" not in st.session_state:
    st.session_state.medidas = cargar_datos(archivos["medidas_1k.csv"], skip_lines=0)
if "pacientes" not in st.session_state:
    st.session_state.pacientes = cargar_datos(archivos["pacientes_1k.csv"], skip_lines=0)
if "registro_login" not in st.session_state:
    st.session_state.registro_login = cargar_datos(archivos["registro_login_1k.csv"], skip_lines=0)
if "sensores" not in st.session_state:
    st.session_state.sensores = cargar_datos(archivos["sensores_500.csv"], skip_lines=0)
if "sesiones" not in st.session_state:
    st.session_state.sesiones = cargar_datos(archivos["sesiones_1k.csv"], skip_lines=0)
if "tipo_sensor" not in st.session_state:
    st.session_state.tipo_sensor = cargar_datos(archivos["tipo_sensor.csv"], skip_lines=0)

# Título de la aplicación
st.title("Análisis de Datos de Salud - CuboLab")

# Botón para actualizar manualmente la base de datos
if st.button("Actualizar Base de Datos"):
    st.session_state.enfermeras = cargar_datos(archivos["enfermeras_1k.csv"], skip_lines=0)
    st.session_state.pacientes_enfermeras = cargar_datos(archivos["pacientes_enfermeras_new.csv"], skip_lines=0)
    st.session_state.centros = cargar_datos(archivos["centros_100.csv"], skip_lines=0)
    st.session_state.intervenciones = cargar_datos(archivos["intervenciones_1k.csv"], skip_lines=0)
    st.session_state.medidas = cargar_datos(archivos["medidas_1k.csv"], skip_lines=0)
    st.session_state.pacientes = cargar_datos(archivos["pacientes_1k.csv"], skip_lines=0)
    st.session_state.registro_login = cargar_datos(archivos["registro_login_1k.csv"], skip_lines=0)
    st.session_state.sensores = cargar_datos(archivos["sensores_500.csv"], skip_lines=0)
    st.session_state.sesiones = cargar_datos(archivos["sesiones_1k.csv"], skip_lines=0)
    st.session_state.tipo_sensor = cargar_datos(archivos["tipo_sensor.csv"], skip_lines=0)
    st.success("Datos actualizados desde archivos locales")

# Menú de selección de tabla
tabla_seleccionada = st.selectbox("Selecciona la tabla para analizar:", 
                                  ["Enfermeras", "Pacientes", "Centros", "Intervenciones", 
                                   "Medidas", "Pacientes_Enfermeras", "RegistroLogin", "Sensores", "Sesiones"])

# Desplegable para filtrar por Centro (aplicable a varias tablas)
centros_disponibles = sorted(st.session_state.centros["nombre"].dropna().unique())
centro_filtro = st.selectbox("Filtrar por Centro:", 
                             ["Todos"] + list(centros_disponibles))

# Extraer el nombre del centro del filtro
nombre_centro_seleccionado = None
id_centro_seleccionado = None
if centro_filtro != "Todos":
    nombre_centro_seleccionado = centro_filtro
    # Obtener el id_centro correspondiente al nombre seleccionado
    id_centro_seleccionado = st.session_state.centros[st.session_state.centros["nombre"] == nombre_centro_seleccionado]["id_centro"].values
    if len(id_centro_seleccionado) > 0:
        id_centro_seleccionado = id_centro_seleccionado[0]
    else:
        id_centro_seleccionado = None

# Mostrar y analizar cada tabla en función de la selección
if tabla_seleccionada == "Enfermeras":
    if id_centro_seleccionado is None:
        enfermeras_data = st.session_state.enfermeras
    else:
        enfermeras_data = st.session_state.enfermeras[st.session_state.enfermeras["id_centro"] == id_centro_seleccionado]
    
    st.write(f"### Tabla: Enfermeras {f'({nombre_centro_seleccionado})' if nombre_centro_seleccionado else ''}")
    st.dataframe(enfermeras_data[["nombre", "id_enfermera", "rol", "edad", "sexo"]], height=400)

    analisis = st.selectbox("Selecciona el análisis para Enfermeras:", 
                            ["Distribución por Sexo", "Distribución por Rol", "Edad por Rol"])

    if analisis == "Distribución por Sexo":
        st.write("Distribución de Enfermeras por Sexo")
        st.bar_chart(enfermeras_data["sexo"].value_counts())

    elif analisis == "Distribución por Rol":
        st.write("Distribución de Enfermeras por Rol")
        st.bar_chart(enfermeras_data["rol"].value_counts())

    elif analisis == "Edad por Rol":
        st.write("Distribución de Edad por Rol")
        chart = alt.Chart(enfermeras_data).mark_boxplot().encode(
            x='rol:N',
            y='edad:Q',
            color='rol:N'
        )
        st.altair_chart(chart, use_container_width=True)

elif tabla_seleccionada == "Pacientes":
    if id_centro_seleccionado is None:
        pacientes_data = st.session_state.pacientes
    else:
        pacientes_data = st.session_state.pacientes[st.session_state.pacientes["id_centro"] == id_centro_seleccionado]
    
    st.write(f"### Tabla: Pacientes {f'({nombre_centro_seleccionado})' if nombre_centro_seleccionado else ''}")
    st.dataframe(pacientes_data[["nombre", "id_paciente", "id_centro", "tieneCubo"]], height=400)

    analisis = st.selectbox("Selecciona el análisis para Pacientes:", 
                            ["Distribución por Centro", "Pacientes con Cubo"])

    if analisis == "Distribución por Centro":
        st.write("Cantidad de Pacientes por Centro")
        st.bar_chart(pacientes_data["id_centro"].value_counts())

    elif analisis == "Pacientes con Cubo":
        st.write("Distribución de Pacientes con o sin Cubo")
        st.bar_chart(pacientes_data["tieneCubo"].value_counts())
elif tabla_seleccionada == "Centros":
    st.write("### Tabla: Centros")
    st.dataframe(st.session_state.centros[["fecha", "nombre", "id_centro"]], height=400)

    analisis = st.selectbox("Selecciona el análisis para Centros:", 
                            ["Centros Activos/Inactivos", "Fechas de Registro", "Pacientes por Centro", "Enfermeras por Centro", "Comparación Pacientes vs Enfermeras por Centro"])

    if analisis == "Centros Activos/Inactivos":
        st.write("Distribución de Centros Activos e Inactivos")
        st.bar_chart(st.session_state.centros["visible"].value_counts())

    elif analisis == "Fechas de Registro":
        st.write("Histograma de Fechas de Registro de los Centros")
        chart = alt.Chart(st.session_state.centros).mark_bar().encode(
            x=alt.X("year(fecha):T", title="Año"),
            y='count()'
        )
        st.altair_chart(chart, use_container_width=True)

    elif analisis == "Pacientes por Centro":
        st.write("Número de Pacientes por Centro")
        pacientes_por_centro = st.session_state.pacientes["id_centro"].value_counts()
        st.bar_chart(pacientes_por_centro)

    elif analisis == "Enfermeras por Centro":
        st.write("Número de Enfermeras por Centro")
        enfermeras_por_centro = st.session_state.enfermeras["id_centro"].value_counts()
        st.bar_chart(enfermeras_por_centro)

    elif analisis == "Comparación Pacientes vs Enfermeras por Centro":
        st.write("### Comparación del Número de Pacientes vs Enfermeras por Centro")

        # Calcular el número de pacientes por centro
        pacientes_por_centro = st.session_state.pacientes["id_centro"].value_counts().rename("Pacientes")
        enfermeras_por_centro = st.session_state.enfermeras["id_centro"].value_counts().rename("Enfermeras")

        # Combinar ambas series en un DataFrame
        comparacion_df = pd.DataFrame({"Pacientes": pacientes_por_centro, "Enfermeras": enfermeras_por_centro}).fillna(0)
        comparacion_df.index.name = "Centro"
        st.write(comparacion_df)

        # Mostrar gráfico de barras apiladas para la comparación
        st.write("**Visualización Comparativa**")
        comparacion_chart = alt.Chart(comparacion_df.reset_index()).transform_fold(
            ["Pacientes", "Enfermeras"],
            as_=["Tipo", "Cantidad"]
        ).mark_bar().encode(
            x=alt.X("Centro:N", title="Centro"),
            y=alt.Y("Cantidad:Q", title="Cantidad"),
            color="Tipo:N"
        )
        st.altair_chart(comparacion_chart, use_container_width=True)

elif tabla_seleccionada == "Intervenciones":
    if id_centro_seleccionado is None:
        intervenciones_data = st.session_state.intervenciones
    else:
        # Filtrar intervenciones por pacientes del centro seleccionado
        pacientes_centro = st.session_state.pacientes[st.session_state.pacientes["id_centro"] == id_centro_seleccionado]["id_paciente"].unique()
        intervenciones_data = st.session_state.intervenciones[st.session_state.intervenciones["id_paciente"].isin(pacientes_centro)]
    
    st.write(f"### Tabla: Intervenciones {f'({nombre_centro_seleccionado})' if nombre_centro_seleccionado else ''}")
    st.dataframe(intervenciones_data, height=400)

    analisis = st.selectbox("Selecciona el análisis para Intervenciones:", 
                            ["Intervenciones por Enfermera", "Intervenciones por Paciente", "Tipos de Intervención"])

    if analisis == "Intervenciones por Enfermera":
        st.write("Distribución de Intervenciones por Enfermera")
        st.bar_chart(intervenciones_data["id_enfermera"].value_counts())

    elif analisis == "Intervenciones por Paciente":
        st.write("Distribución de Intervenciones por Paciente")
        st.bar_chart(intervenciones_data["id_paciente"].value_counts())

    elif analisis == "Tipos de Intervención":
        st.write("Distribución de Tipos de Intervención")
        st.bar_chart(intervenciones_data["intervencion"].value_counts())


elif tabla_seleccionada == "Medidas":
    if id_centro_seleccionado is None:
        medidas_data = st.session_state.medidas
    else:
        # Filtrar medidas por pacientes del centro seleccionado
        pacientes_centro = st.session_state.pacientes[st.session_state.pacientes["id_centro"] == id_centro_seleccionado]["id_paciente"].unique()
        medidas_data = st.session_state.medidas[st.session_state.medidas["id_paciente"].isin(pacientes_centro)]
    
    st.write(f"### Tabla: Medidas {f'({nombre_centro_seleccionado})' if nombre_centro_seleccionado else ''}")
    st.dataframe(medidas_data[["fecha", "id_medida", "id_paciente", "emocion", "bateria"]], height=400)

    # Desplegable para seleccionar entre análisis y predicciones
    opcion = st.selectbox("Selecciona la opción:", ["Análisis", "Predicción"])

    if opcion == "Análisis":
        st.write("### Análisis de Medidas")

        # Análisis general de "Estado Emocional de Pacientes" y "Niveles de Batería"
        analisis = st.selectbox("Selecciona el análisis para Medidas:", 
                                ["Estado Emocional de Pacientes", "Niveles de Batería", "Evolución por Paciente"])

        if analisis == "Estado Emocional de Pacientes":
            st.write("Distribución del Estado Emocional de los Pacientes")
            st.bar_chart(medidas_data["emocion"].value_counts())

        elif analisis == "Niveles de Batería":
            st.write("Distribución de los Niveles de Batería de los Sensores")
            chart = alt.Chart(medidas_data).mark_bar().encode(
                x=alt.X("bateria:Q", bin=True),
                y='count()'
            )
            st.altair_chart(chart, use_container_width=True)

        elif analisis == "Evolución por Paciente":
            # Input para seleccionar el id del paciente
            ids_validos = sorted(medidas_data["id_paciente"].dropna().unique())
            
            if len(ids_validos) == 0:
                st.warning("No hay medidas disponibles para el centro seleccionado.")
            else:
                id_paciente = st.selectbox("Introduce el ID del paciente:", ids_validos)
                
                # Filtrar los datos por id_paciente
                medidas_filtradas = medidas_data[medidas_data["id_paciente"] == id_paciente]
                
                if medidas_filtradas.empty:
                    st.write("No se encontraron medidas para el ID de paciente especificado.")
                else:
                    # Convertir la columna de fecha a datetime
                    medidas_filtradas = medidas_filtradas.copy()
                    medidas_filtradas["fecha"] = pd.to_datetime(medidas_filtradas["fecha"], errors="coerce")
                    medidas_filtradas = medidas_filtradas.sort_values("fecha")

                    # Gráfico de líneas del estado emocional a lo largo del tiempo
                    st.write("**Evolución del Estado Emocional a lo largo del tiempo**")
                    chart = alt.Chart(medidas_filtradas).mark_line().encode(
                        x="fecha:T",
                        y="emocion:Q"
                    )
                    st.altair_chart(chart, use_container_width=True)

    elif opcion == "Predicción":
        st.write("### Predicción de Medidas por Paciente")

        # Seleccionar el id del paciente
        ids_validos = sorted(medidas_data["id_paciente"].dropna().unique())
        
        if len(ids_validos) == 0:
            st.warning("No hay medidas disponibles para el centro seleccionado.")
        else:
            id_paciente = st.selectbox(
                "Selecciona el ID del paciente:",
                ids_validos
            )
            st.write("Seleccionaste:", id_paciente)

            # Filtrar los datos por id_paciente
            medidas_filtradas = medidas_data[
            medidas_data["id_paciente"] == id_paciente
            ]

            # Convertir fecha a datetime
            medidas_filtradas = medidas_filtradas.copy()
            medidas_filtradas["fecha"] = pd.to_datetime(medidas_filtradas["fecha"], errors="coerce")

            # Código ChatGPT
            medidas_por_fecha = (
                medidas_filtradas
                .set_index("fecha")["emocion"]
                .resample("D")
                .mean()
                .interpolate(method="time")
            )
            # Serie temporal diaria
            serie = medidas_por_fecha.dropna()

            if len(serie) < 3:
                st.warning("No hay suficientes datos para realizar la predicción.")
            else:
                # Entrenar modelo ARIMA
                modelo = ARIMA(serie, order=(1, 1, 1))
                ajuste = modelo.fit()

                # Predicción 30 días
                pred = ajuste.get_forecast(steps=30)
                pred_media = pred.predicted_mean
                pred_intervalos = pred.conf_int()

                st.write("**Predicción del Estado Emocional (ARIMA)**")

                st.line_chart(
                    pd.concat([
                        serie.rename("Histórico"),
                        pred_media.rename("Predicción")
                    ])
                )

elif tabla_seleccionada == "Pacientes_Enfermeras":
    if id_centro_seleccionado is None:
        pacientes_enfermeras_data = st.session_state.pacientes_enfermeras
    else:
        # Filtrar por pacientes del centro seleccionado
        pacientes_centro = st.session_state.pacientes[st.session_state.pacientes["id_centro"] == id_centro_seleccionado]["id_paciente"].unique()
        pacientes_enfermeras_data = st.session_state.pacientes_enfermeras[st.session_state.pacientes_enfermeras["id_paciente"].isin(pacientes_centro)]
    
    st.write(f"### Tabla: Pacientes_Enfermeras {f'({nombre_centro_seleccionado})' if nombre_centro_seleccionado else ''}")
    st.dataframe(pacientes_enfermeras_data, height=400)

    analisis = st.selectbox("Selecciona el análisis para Pacientes_Enfermeras:", 
                            ["Cantidad de Pacientes por Enfermera", "Enfermeras Asignadas a Pacientes"])

    if analisis == "Cantidad de Pacientes por Enfermera":
        st.write("Distribución de Pacientes por Enfermera")
        st.bar_chart(pacientes_enfermeras_data["id_enfermera"].value_counts())

    elif analisis == "Enfermeras Asignadas a Pacientes":
        st.write("Distribución de Enfermeras asignadas por Paciente")
        st.bar_chart(pacientes_enfermeras_data["id_paciente"].value_counts())

elif tabla_seleccionada == "RegistroLogin":
    if id_centro_seleccionado is None:
        registro_login_data = st.session_state.registro_login
    else:
        # Filtrar por enfermeras del centro seleccionado
        enfermeras_centro = st.session_state.enfermeras[st.session_state.enfermeras["id_centro"] == id_centro_seleccionado]["id_enfermera"].unique()
        registro_login_data = st.session_state.registro_login[st.session_state.registro_login["id_enfermera"].isin(enfermeras_centro)]
    
    st.write(f"### Tabla: RegistroLogin {f'({nombre_centro_seleccionado})' if nombre_centro_seleccionado else ''}")
    st.dataframe(registro_login_data, height=400)
    
    analisis = st.selectbox("Selecciona el análisis para RegistroLogin:", 
                            ["Frecuencia de Logins por Enfermera", "Distribución de IPs"])

    if analisis == "Frecuencia de Logins por Enfermera":
        st.write("Cantidad de Logins por Enfermera")
        st.bar_chart(registro_login_data["id_enfermera"].value_counts())

    elif analisis == "Distribución de IPs":
        st.write("Distribución de IPs utilizadas")
        st.bar_chart(registro_login_data["ip_address"].value_counts())

elif tabla_seleccionada == "Sensores":
    if id_centro_seleccionado is None:
        sensores_data = st.session_state.sensores
    else:
        sensores_data = st.session_state.sensores[st.session_state.sensores["id_centro"] == id_centro_seleccionado]
    
    st.write(f"### Tabla: Sensores {f'({nombre_centro_seleccionado})' if nombre_centro_seleccionado else ''}")
    st.dataframe(sensores_data[["nick", "MAC", "id_centro", "id_paciente", "tipo", "bateria"]], height=400)

    analisis = st.selectbox("Selecciona el análisis para Sensores:", 
                            ["Sensores Activos/Inactivos", "Distribución por Tipo de Sensor", "Niveles de Batería"])

    if analisis == "Sensores Activos/Inactivos":
        st.write("Cantidad de Sensores Activos e Inactivos")
        st.bar_chart(sensores_data["activo"].value_counts())

    elif analisis == "Distribución por Tipo de Sensor":
        st.write("Distribución por Tipo de Sensor")
        st.bar_chart(sensores_data["tipo"].value_counts())

    elif analisis == "Niveles de Batería":
        st.write("Distribución de los Niveles de Batería")
        chart = alt.Chart(sensores_data).mark_bar().encode(
            x=alt.X("bateria:Q", bin=True),
            y='count()'
        )
        st.altair_chart(chart, use_container_width=True)

elif tabla_seleccionada == "Sesiones":
        if id_centro_seleccionado is None:
            sesiones_data = st.session_state.sesiones
        else:
            # Filtrar por enfermeras del centro seleccionado
            enfermeras_centro = st.session_state.enfermeras[st.session_state.enfermeras["id_centro"] == id_centro_seleccionado]["id_enfermera"].unique()
            sesiones_data = st.session_state.sesiones[st.session_state.sesiones["id_enfermera"].isin(enfermeras_centro)]
        
        st.write(f"### Tabla: Sesiones {f'({nombre_centro_seleccionado})' if nombre_centro_seleccionado else ''}")
        st.dataframe(sesiones_data[["Id_Sesion", "id_enfermera"]], height=400)

        analisis = st.selectbox("Selecciona el análisis para Sesiones:", 
                                ["Frecuencia de Sesiones por Enfermera"])

        if analisis == "Frecuencia de Sesiones por Enfermera":
            st.write("Cantidad de Sesiones por Enfermera")
            st.bar_chart(sesiones_data["id_enfermera"].value_counts())


# streamlit run interfaz_tablas_analisis.py