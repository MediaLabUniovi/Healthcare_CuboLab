import streamlit as st
import pandas as pd
import requests
from io import StringIO
import altair as alt
from statsmodels.tsa.holtwinters import ExponentialSmoothing

# URLs de descarga directa desde OSF
urls = {
    "enfermeras_1k.csv": "https://osf.io/download/6729eeb4b7aee73e2402757d/",
    "pacientes_enfermeras_new.csv": "https://osf.io/download/672a0d0f1728a8447dbaa565/",
    "alertas_1k.csv": "https://osf.io/download/672c85e126c2c04ce0f7430d/",
    "centros_100.csv": "https://osf.io/download/672c85e409c7f0c228f5ca1d/",
    "intervenciones_1k.csv": "https://osf.io/download/6729eebbd2d4ccdb68dcca8f/",
    "medidas_1k.csv": "https://osf.io/download/6729eec61bad4ca7a3baa1aa/",
    "pacientes_1k.csv": "https://osf.io/download/672c85e926c2c04ce0f74311/",
    "registro_login_1k.csv": "https://osf.io/download/672c861c2f97981570f742b8/",
    "sensores_500.csv": "https://osf.io/download/672c862303398a567bba9fed/",
    "sesiones_1k.csv": "https://osf.io/download/672c8629356efed2b2ba9dff/",
    "tipo_sensor.csv": "https://osf.io/download/672c862da753399c3af5ce64/"
}

# Función para cargar los datos sin cache
def cargar_datos(url, skip_lines):
    response = requests.get(url)
    response.raise_for_status()
    data_str = response.text
    return pd.read_csv(StringIO(data_str), skiprows=skip_lines)

# Inicializar el estado de los datos al inicio
if "enfermeras" not in st.session_state:
    st.session_state.enfermeras = cargar_datos(urls["enfermeras_1k.csv"], skip_lines=3)
if "pacientes_enfermeras" not in st.session_state:
    st.session_state.pacientes_enfermeras = cargar_datos(urls["pacientes_enfermeras_new.csv"], skip_lines=3)
if "alertas" not in st.session_state:
    st.session_state.alertas = cargar_datos(urls["alertas_1k.csv"], skip_lines=3)
if "centros" not in st.session_state:
    st.session_state.centros = cargar_datos(urls["centros_100.csv"], skip_lines=3)
if "intervenciones" not in st.session_state:
    st.session_state.intervenciones = cargar_datos(urls["intervenciones_1k.csv"], skip_lines=3)
if "medidas" not in st.session_state:
    st.session_state.medidas = cargar_datos(urls["medidas_1k.csv"], skip_lines=3)
if "pacientes" not in st.session_state:
    st.session_state.pacientes = cargar_datos(urls["pacientes_1k.csv"], skip_lines=3)
if "registro_login" not in st.session_state:
    st.session_state.registro_login = cargar_datos(urls["registro_login_1k.csv"], skip_lines=3)
if "sensores" not in st.session_state:
    st.session_state.sensores = cargar_datos(urls["sensores_500.csv"], skip_lines=3)
if "sesiones" not in st.session_state:
    st.session_state.sesiones = cargar_datos(urls["sesiones_1k.csv"], skip_lines=3)
if "tipo_sensor" not in st.session_state:
    st.session_state.tipo_sensor = cargar_datos(urls["tipo_sensor.csv"], skip_lines=3)

# Título de la aplicación
st.title("Análisis de Datos de Salud - Cubolab")

# Botón para actualizar manualmente la base de datos
if st.button("Actualizar Base de Datos"):
    st.session_state.enfermeras = cargar_datos(urls["enfermeras_1k.csv"], skip_lines=3)
    st.session_state.pacientes_enfermeras = cargar_datos(urls["pacientes_enfermeras_new.csv"], skip_lines=3)
    st.session_state.alertas = cargar_datos(urls["alertas_1k.csv"], skip_lines=3)
    st.session_state.centros = cargar_datos(urls["centros_100.csv"], skip_lines=3)
    st.session_state.intervenciones = cargar_datos(urls["intervenciones_1k.csv"], skip_lines=3)
    st.session_state.medidas = cargar_datos(urls["medidas_1k.csv"], skip_lines=3)
    st.session_state.pacientes = cargar_datos(urls["pacientes_1k.csv"], skip_lines=3)
    st.session_state.registro_login = cargar_datos(urls["registro_login_1k.csv"], skip_lines=3)
    st.session_state.sensores = cargar_datos(urls["sensores_500.csv"], skip_lines=3)
    st.session_state.sesiones = cargar_datos(urls["sesiones_1k.csv"], skip_lines=3)
    st.session_state.tipo_sensor = cargar_datos(urls["tipo_sensor.csv"], skip_lines=3)
    st.success("Datos actualizados desde el servidor OSF")

# Menú de selección de tabla
tabla_seleccionada = st.selectbox("Selecciona la tabla para analizar:", 
                                  ["Enfermeras", "Pacientes", "Alertas", "Centros", "Intervenciones", 
                                   "Medidas", "Pacientes_Enfermeras", "RegistroLogin", "Sensores", "Sesiones", "TipoSensor"])



# Mostrar y analizar cada tabla en función de la selección
if tabla_seleccionada == "Enfermeras":
    st.write("### Tabla: Enfermeras")
    st.dataframe(st.session_state.enfermeras.head())

    analisis = st.selectbox("Selecciona el análisis para Enfermeras:", 
                            ["Distribución por Sexo", "Distribución por Rol", "Edad por Rol"])

    if analisis == "Distribución por Sexo":
        st.write("Distribución de Enfermeras por Sexo")
        st.bar_chart(st.session_state.enfermeras["sexo"].value_counts())

    elif analisis == "Distribución por Rol":
        st.write("Distribución de Enfermeras por Rol")
        st.bar_chart(st.session_state.enfermeras["rol"].value_counts())

    elif analisis == "Edad por Rol":
        st.write("Distribución de Edad por Rol")
        chart = alt.Chart(st.session_state.enfermeras).mark_boxplot().encode(
            x='rol:N',
            y='edad:Q',
            color='rol:N'
        )
        st.altair_chart(chart, use_container_width=True)

elif tabla_seleccionada == "Pacientes":
    st.write("### Tabla: Pacientes")
    st.dataframe(st.session_state.pacientes.head())

    analisis = st.selectbox("Selecciona el análisis para Pacientes:", 
                            ["Distribución por Centro", "Pacientes con Cubo"])

    if analisis == "Distribución por Centro":
        st.write("Cantidad de Pacientes por Centro")
        st.bar_chart(st.session_state.pacientes["id_centro"].value_counts())

    elif analisis == "Pacientes con Cubo":
        st.write("Distribución de Pacientes con o sin Cubo")
        st.bar_chart(st.session_state.pacientes["tieneCubo"].value_counts())

elif tabla_seleccionada == "Alertas":
    st.write("### Tabla: Alertas")
    st.dataframe(st.session_state.alertas.head())

    analisis = st.selectbox("Selecciona el análisis para Alertas:", 
                            ["Alertas por Sensor", "Alertas por Paciente", "Paciente con más Alertas", 
                             "Picos de Alertas por Fecha", "Centro con más Alertas"])

    if analisis == "Alertas por Sensor":
        st.write("Distribución de Alertas por Sensor")
        st.bar_chart(st.session_state.alertas["sensor"].value_counts())

    elif analisis == "Alertas por Paciente":
        st.write("Cantidad de Alertas por Paciente")
        st.bar_chart(st.session_state.alertas["id_usuario"].value_counts())

    elif analisis == "Paciente con más Alertas":
        paciente_mas_alertas = st.session_state.alertas["id_usuario"].value_counts().idxmax()
        st.write(f"El paciente con más alertas es: {paciente_mas_alertas}")
        st.bar_chart(st.session_state.alertas["id_usuario"].value_counts())

    elif analisis == "Picos de Alertas por Fecha":
        # Convertir la columna de fecha a formato datetime si aún no está en ese formato
        st.session_state.alertas["fecha"] = pd.to_datetime(st.session_state.alertas["fecha"], errors="coerce")

        # Análisis por día de la semana
        st.session_state.alertas["dia_semana"] = st.session_state.alertas["fecha"].dt.day_name()
        st.write("Distribución de Alertas por Día de la Semana")
        st.bar_chart(st.session_state.alertas["dia_semana"].value_counts())

        # Análisis por hora del día
        st.session_state.alertas["hora"] = st.session_state.alertas["fecha"].dt.hour
        st.write("Distribución de Alertas por Hora del Día")
        st.bar_chart(st.session_state.alertas["hora"].value_counts())

        # Análisis por mes
        st.session_state.alertas["mes"] = st.session_state.alertas["fecha"].dt.month
        st.write("Distribución de Alertas por Mes")
        st.bar_chart(st.session_state.alertas["mes"].value_counts())

    elif analisis == "Centro con más Alertas":
        # Asegúrate de que la tabla tenga una columna que relacione cada alerta con un centro.
        if "id_centro" in st.session_state.alertas.columns:
            st.write("Distribución de Alertas por Centro")
            st.bar_chart(st.session_state.alertas["id_centro"].value_counts())
        else:
            st.write("No se encontró una columna de centro en la tabla de alertas.")

elif tabla_seleccionada == "Centros":
    st.write("### Tabla: Centros")
    st.dataframe(st.session_state.centros.head())

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
    st.write("### Tabla: Intervenciones")
    st.dataframe(st.session_state.intervenciones.head())

    analisis = st.selectbox("Selecciona el análisis para Intervenciones:", 
                            ["Intervenciones por Rol", "Intervenciones por Centro"])

    if analisis == "Intervenciones por Rol":
        st.write("Distribución de Intervenciones por Rol de la Enfermera")
        st.bar_chart(st.session_state.intervenciones["rol"].value_counts())

    elif analisis == "Intervenciones por Centro":
        st.write("Distribución de Intervenciones por Centro")
        st.bar_chart(st.session_state.intervenciones["id_centro"].value_counts())


elif tabla_seleccionada == "Medidas":
    st.write("### Tabla: Medidas")
    st.dataframe(st.session_state.medidas.head())

    # Desplegable para seleccionar entre análisis y predicciones
    opcion = st.selectbox("Selecciona la opción:", ["Análisis", "Predicción"])

    if opcion == "Análisis":
        st.write("### Análisis de Medidas")

        # Análisis general de "Estado Emocional de Pacientes" y "Niveles de Batería"
        analisis = st.selectbox("Selecciona el análisis para Medidas:", 
                                ["Estado Emocional de Pacientes", "Niveles de Batería", "Evolución por Paciente"])

        if analisis == "Estado Emocional de Pacientes":
            st.write("Distribución del Estado Emocional de los Pacientes")
            st.bar_chart(st.session_state.medidas["emocion"].value_counts())

        elif analisis == "Niveles de Batería":
            st.write("Distribución de los Niveles de Batería de los Sensores")
            chart = alt.Chart(st.session_state.medidas).mark_bar().encode(
                x=alt.X("bateria:Q", bin=True),
                y='count()'
            )
            st.altair_chart(chart, use_container_width=True)

        elif analisis == "Evolución por Paciente":
            # Input para seleccionar el id del paciente
            id_paciente = st.number_input("Introduce el ID del paciente:", min_value=int(st.session_state.medidas["id_paciente"].min()), max_value=int(st.session_state.medidas["id_paciente"].max()), step=1)
            
            # Filtrar los datos por id_paciente
            medidas_filtradas = st.session_state.medidas[st.session_state.medidas["id_paciente"] == id_paciente]
            
            if medidas_filtradas.empty:
                st.write("No se encontraron medidas para el ID de paciente especificado.")
            else:
                # Convertir la columna de fecha a datetime
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

        # Input para seleccionar el id del paciente
        id_paciente = st.number_input("Introduce el ID del paciente para la predicción:", min_value=int(st.session_state.medidas["id_paciente"].min()), max_value=int(st.session_state.medidas["id_paciente"].max()), step=1)
        
        # Filtrar los datos por id_paciente
        medidas_filtradas = st.session_state.medidas[st.session_state.medidas["id_paciente"] == id_paciente]
        
        if medidas_filtradas.empty:
            st.write("No se encontraron medidas para el ID de paciente especificado.")
        else:
            # Convertir la columna de fecha a datetime
            medidas_filtradas["fecha"] = pd.to_datetime(medidas_filtradas["fecha"], errors="coerce")
            medidas_filtradas = medidas_filtradas.sort_values("fecha")

            # Preparar los datos para el modelo de series temporales
            medidas_por_fecha = medidas_filtradas.set_index("fecha")["emocion"].resample("D").mean().fillna(0)

            # Modelo de series temporales: Suavizado exponencial
            modelo = ExponentialSmoothing(medidas_por_fecha, trend="add", seasonal=None)
            ajuste = modelo.fit()

            # Predicción para los próximos 30 días
            prediccion = ajuste.forecast(30)

            # Gráfico de la predicción
            st.write("**Predicción del Estado Emocional para los próximos 30 días**")
            st.line_chart(prediccion)


elif tabla_seleccionada == "Pacientes_Enfermeras":
    st.write("### Tabla: Pacientes_Enfermeras")
    st.dataframe(st.session_state.pacientes_enfermeras.head())

    analisis = st.selectbox("Selecciona el análisis para Pacientes_Enfermeras:", 
                            ["Cantidad de Pacientes por Enfermera", "Enfermeras Asignadas a Pacientes"])

    if analisis == "Cantidad de Pacientes por Enfermera":
        st.write("Distribución de Pacientes por Enfermera")
        st.bar_chart(st.session_state.pacientes_enfermeras["id_enfermera"].value_counts())

    elif analisis == "Enfermeras Asignadas a Pacientes":
        st.write("Distribución de Enfermeras asignadas por Paciente")
        st.bar_chart(st.session_state.pacientes_enfermeras["id_paciente"].value_counts())

elif tabla_seleccionada == "RegistroLogin":
    st.write("### Tabla: RegistroLogin")
    st.dataframe(st.session_state.registro_login.head())
    
    analisis = st.selectbox("Selecciona el análisis para RegistroLogin:", 
                            ["Frecuencia de Logins por Enfermera", "Distribución de IPs"])

    if analisis == "Frecuencia de Logins por Enfermera":
        st.write("Cantidad de Logins por Enfermera")
        st.bar_chart(st.session_state.registro_login["id_enfermera"].value_counts())

    elif analisis == "Distribución de IPs":
        st.write("Distribución de IPs utilizadas")
        st.bar_chart(st.session_state.registro_login["ip_address"].value_counts())

elif tabla_seleccionada == "Sensores":
    st.write("### Tabla: Sensores")
    st.dataframe(st.session_state.sensores.head())

    analisis = st.selectbox("Selecciona el análisis para Sensores:", 
                            ["Sensores Activos/Inactivos", "Distribución por Tipo de Sensor", "Niveles de Batería"])

    if analisis == "Sensores Activos/Inactivos":
        st.write("Cantidad de Sensores Activos e Inactivos")
        st.bar_chart(st.session_state.sensores["activo"].value_counts())

    elif analisis == "Distribución por Tipo de Sensor":
        st.write("Distribución por Tipo de Sensor")
        st.bar_chart(st.session_state.sensores["tipo"].value_counts())

    elif analisis == "Niveles de Batería":
        st.write("Distribución de los Niveles de Batería")
        chart = alt.Chart(st.session_state.sensores).mark_bar().encode(
            x=alt.X("bateria:Q", bin=True),
            y='count()'
        )
        st.altair_chart(chart, use_container_width=True)


elif tabla_seleccionada == "Sesiones":
        st.write("### Tabla: Sesiones")
        st.dataframe(st.session_state.sesiones.head())

        analisis = st.selectbox("Selecciona el análisis para Sesiones:", 
                                ["Frecuencia de Sesiones por Enfermera"])

        if analisis == "Frecuencia de Sesiones por Enfermera":
            st.write("Cantidad de Sesiones por Enfermera")
            st.bar_chart(st.session_state.sesiones["id_enfermera"].value_counts())

elif tabla_seleccionada == "TipoSensor":
        st.write("### Tabla: TipoSensor")
        st.dataframe(st.session_state.tipo_sensor.head())

        analisis = st.selectbox("Selecciona el análisis para TipoSensor:", 
                                ["Distribución de Tipos de Sensores"])

        if analisis == "Distribución de Tipos de Sensores":
            st.write("Distribución de Tipos de Sensores")
            st.bar_chart(st.session_state.tipo_sensor["nombre"].value_counts())


# streamlit run appentera.py
