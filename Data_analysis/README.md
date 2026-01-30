# CuboLab - Interfaz de Análisis de Datos de Enfermería

## 📋 Descripción

Conjunto de aplicaciones desarrolladas con **Streamlit** que permiten analizar y visualizar datos relacionados con enfermeras, pacientes, residentes y los cubos empleados en el laboratorio de salud CuboLab.

Las aplicaciones están diseñadas para facilitar la interacción y actualización de los datos almacenados en servidores externos. Los usuarios pueden seleccionar diferentes tablas, realizar análisis específicos y visualizar la información de manera clara y comprensible. Se incluyen predicciones de emociones para días futuros y análisis de texto de comentarios/intervenciones.

Los datos provienen de estudios realizados con residentes de Medialab durante diciembre de 2024 y posteriores.

---

## 🎯 Características Principales

- **Actualización de la base de datos**: Botón para actualizar los datos con un solo clic directamente desde servidores externos.
- **Visualización de datos**: Muestra tablas con información relevante (enfermeras, pacientes, personal, etc.) con múltiples columnas de análisis.
- **Interfaz intuitiva**: Desplegables y selectores para navegar fácilmente entre distintos análisis y tablas.
- **Análisis y predicción**: Opciones de análisis específicos con gráficos interactivos y predicciones de emociones usando modelos SARIMAX.
- **Análisis de texto**: Visualización mediante nubes de palabras según la emoción a partir de comentarios/intervenciones.
- **Gráficas interactivas**: Gráficas escalonadas con Plotly que permiten visualizar datos históricos y predicciones en tiempo real.

---

## 📁 Estructura del Proyecto

```
Data_analysis/
├── Interfaz_Emociones/
│   ├── interfaz_emociones_descritas.py    # Script principal para análisis de emociones
│   └── requirements.txt                   # Dependencias Python
├── Interfaz_General/
│   ├── interfaz_tablas_analisis.py       # Análisis general de tablas
│   ├── requirements.txt
│   └── data/                              # Archivos CSV con datos
│       ├── auditlog.csv
│       ├── centros.csv
│       ├── enfermeras.csv
│       ├── intervenciones.csv
│       ├── medidas.csv
│       ├── opiniones.csv
│       ├── pacientes.csv
│       └── [otros archivos de datos]
├── Interfaz_Residentes/
│   ├── interfaz_real_residentes.py       # Análisis de residentes
│   ├── requirements.txt
│   └── df_real_descargado.csv
├── notebook1.ipynb, notebook2.ipynb, notebook3.ipynb  # Análisis exploratorios
├── enlaces_app.txt
└── README.md
```

---

## 🚀 Guía de Instalación y Ejecución

### Requisitos Previos

- Python 3.7 o superior
- pip (gestor de paquetes de Python)
- Git (opcional, para clonar el repositorio)

### Pasos de Instalación

1. **Clonar o descargar el repositorio**:
   ```bash
   git clone <URL_del_repositorio>
   cd Data_analysis
   ```

2. **Crear un entorno virtual** (recomendado):
   ```bash
   python -m venv venv
   source venv/bin/activate  # En macOS/Linux
   # o
   venv\Scripts\activate  # En Windows
   ```

3. **Instalar las dependencias** según la interfaz que desees usar:

   **Para la Interfaz de Emociones**:
   ```bash
   cd Interfaz_Emociones
   pip install -r requirements.txt
   pip install streamlit  # Si no está incluido
   ```

   **Para la Interfaz General**:
   ```bash
   cd Interfaz_General
   pip install -r requirements.txt
   pip install streamlit
   ```

   **Para la Interfaz de Residentes**:
   ```bash
   cd Interfaz_Residentes
   pip install -r requirements.txt
   pip install streamlit
   ```

### Ejecución de los Scripts

Cada interfaz se ejecuta como una aplicación Streamlit independiente:

**Interfaz de Emociones**:
```bash
streamlit run Interfaz_Emociones/interfaz_emociones_descritas.py
```

**Interfaz General**:
```bash
streamlit run Interfaz_General/interfaz_tablas_analisis.py
```

**Interfaz de Residentes**:
```bash
streamlit run Interfaz_Residentes/interfaz_real_residentes.py
```

Una vez ejecutado, Streamlit abrirá una ventana en tu navegador (generalmente en `http://localhost:8501`).

---

## 📊 Interfaz de Emociones (`interfaz_emociones_descritas.py`)

### ¿Qué hace?

Este script proporciona un análisis interactivo de las emociones registradas de usuarios/pacientes. Permite:

- **Seleccionar un usuario** de la lista disponible
- **Visualizar datos históricos** de emociones mediante gráficas escalonadas
- **Ver intervenciones/descripciones** asociadas a cada registro de emoción (al pasar el cursor)
- **Predecir emociones futuras** usando modelos SARIMAX para los próximos 1-30 días
- **Comparar gráficamente** datos históricos con predicciones
- **Descargar tabla de predicciones** para análisis posterior

### Dependencias Principales

- **streamlit**: Framework para crear interfaces web interactivas
- **pandas**: Análisis y manipulación de datos
- **plotly**: Gráficas interactivas
- **statsmodels**: Modelos estadísticos avanzados (SARIMAX para predicciones)
- **requests**: Descargar datos desde servidores externos

### Flujo de Uso

1. Ejecutar el script con `streamlit run`
2. Seleccionar un usuario del desplegable
3. Visualizar la gráfica histórica de emociones
4. Ajustar el número de días a predecir (slider)
5. Ver las predicciones en la gráfica actualizada y en la tabla

### Fuente de Datos

Los datos se descargan directamente desde un servidor OSF (Open Science Framework) mediante URLs específicas configuradas en el script.

---

## � Interfaz General (`interfaz_tablas_analisis.py`)

### ¿Qué hace?

Este script proporciona un análisis completo y flexible de múltiples tablas de datos en la base de datos de CuboLab. Permite:

- **Seleccionar y visualizar diferentes tablas**: Enfermeras, Pacientes, Centros, Intervenciones, Medidas, Sesiones, Sensores, etc.
- **Filtrar por Centro**: Opción para filtrar todos los análisis por un centro específico
- **Análisis estadísticos específicos**: Cada tabla tiene análisis customizados (distribuciones, gráficos comparativos, evoluciones temporales)
- **Análisis de Intervenciones**: Visualización de intervenciones por enfermera, paciente o tipo
- **Análisis de Medidas**: Estado emocional de pacientes, niveles de batería, evolución temporal
- **Predicciones**: Modelos ARIMA y Exponential Smoothing para predecir valores futuros
- **Filtrado de datos**: Actualización dinámica de datos locales con botón de refresh

### Dependencias Principales

- **streamlit**: Framework para crear interfaces web interactivas
- **pandas**: Análisis y manipulación de datos
- **altair**: Gráficas interactivas y visualizaciones
- **statsmodels**: Modelos estadísticos para predicciones (ARIMA, ExponentialSmoothing)

### Tablas Disponibles

La interfaz permite analizar las siguientes tablas:

| Tabla | Descripción | Análisis Disponibles |
|-------|-------------|-------------------|
| **Enfermeras** | Personal de enfermería | Distribución por sexo, rol, edad por rol |
| **Pacientes** | Información de pacientes | Distribución por centro, pacientes con/sin cubo |
| **Centros** | Centros de salud | Activos/inactivos, fechas registro, comparación pacientes vs enfermeras |
| **Intervenciones** | Intervenciones realizadas | Por enfermera, por paciente, tipos |
| **Medidas** | Mediciones de sensores | Emocional, batería, evolución temporal, predicciones |
| **Pacientes-Enfermeras** | Relaciones entre pacientes y enfermeras | Tabla de relaciones |
| **Sesiones** | Sesiones registradas | Análisis temporal |
| **Sensores** | Información de sensores | Tipos y estado |

### Flujo de Uso

1. Ejecutar el script con `streamlit run`
2. Seleccionar una tabla del desplegable principal
3. Opcionalmente, filtrar por centro
4. Elegir un análisis específico de la tabla seleccionada
5. Visualizar gráficos y datos interactivos

### Fuente de Datos

Los datos se cargan desde archivos CSV locales almacenados en la carpeta `/data`:
- enfermeras.csv
- pacientes.csv
- centros.csv
- intervenciones.csv
- medidas.csv
- Y otros archivos de datos

---

## 📊 Interfaz de Residentes (`interfaz_real_residentes.py`)

### ¿Qué hace?

Este script combina análisis de emociones con procesamiento avanzado de lenguaje natural (NLP) para analizar comentarios de residentes. Proporciona:

- **Análisis temporal de emociones**: Gráficas escalonadas de emociones históricas por usuario
- **Predicción de emociones futuras**: Modelos SARIMAX para predecir emociones 1-30 días en el futuro
- **Análisis de comentarios**: Relación entre palabras y emociones mediante TF-IDF
- **Visualización de patrones**: Heatmaps mostrando palabras dominantes por emoción
- **Nubes de palabras**: WordCloud generadas automáticamente por cada emoción
- **Clustering de comentarios**: Agrupación automática de comentarios similares mediante K-Means y PCA
- **Descarga de resultados**: Exportar análisis clustereados a CSV

### Dependencias Principales

- **streamlit**: Framework para interfaces web
- **pandas & numpy**: Manipulación de datos
- **plotly**: Gráficas interactivas
- **statsmodels**: Modelos SARIMAX para predicciones
- **scikit-learn**: TF-IDF, PCA, K-Means clustering
- **wordcloud**: Generación de nubes de palabras
- **matplotlib & seaborn**: Visualizaciones estáticas (heatmaps, gráficos)

### Funcionalidades Principales

#### 1. Análisis Temporal
- Seleccionar usuario específico
- Visualizar línea temporal de emociones
- Ver comentarios asociados al pasar el cursor

#### 2. Predicción SARIMAX
- Predicción configurable (1-30 días)
- Gráficas comparativas histórico + predicción
- Tabla descargable con predicciones

#### 3. Análisis de Texto
- **TF-IDF Vectorization**: Extrae palabras clave por emoción
- **Heatmap**: Matriz de palabras vs emociones mostrando relevancia
- **WordCloud**: Visualización estética de palabras más frecuentes por emoción
- **Clustering**: Agrupa comentarios similares mediante K-Means (3 clusters)

### Flujo de Uso

1. Ejecutar el script con `streamlit run`
2. Seleccionar un usuario del desplegable
3. Visualizar gráfica histórica de emociones
4. Configurar días a predecir y ver predicciones
5. Explorar análisis de comentarios:
   - Ver relación de palabras y emociones
   - Visualizar heatmap de términos por emoción
   - Examinar nubes de palabras por emoción
   - Ver clustering de comentarios
6. Descargar resultados en CSV

### Preprocesamiento de Texto

El script realiza preprocesamiento automático:
- Conversión a minúsculas
- Eliminación de caracteres especiales
- Limpieza de espacios en blanco
- Filtrado de comentarios vacíos

### Fuente de Datos

Los datos se cargan desde un archivo CSV local:
- `df_real_descargado.csv`: Contiene datos de residentes con columnas `id_paciente`, `fecha`, `emocion` y `comentario`

---

## �🔧 Configuración y Personalización

### Modificar URLs de Datos

En `interfaz_emociones_descritas.py`, edita el diccionario `urls` para cambiar la fuente de datos:

```python
urls = {
    "emociones.csv": "https://osf.io/download/<tu-id-osf>/",
}
```

### Ajustar Parámetros de Predicción

Puedes modificar los parámetros del modelo SARIMAX en el script:

```python
modelo = SARIMAX(emociones_diarias, order=(1, 1, 1), seasonal_order=(0, 1, 1, 7))
```

---

## 📝 Notas Importantes

- Los datos se cargan desde servidores externos, por lo que se requiere **conexión a internet**.
- El botón "Actualizar Base de Datos" permite refrescar los datos sin reiniciar la aplicación.
- Las predicciones están basadas en patrones históricos y pueden no ser 100% precisas.
- Los datos utilizados provienen de estudios en Medialab durante diciembre de 2024.

---

## 👥 Autor/es

Desarrollado por el equipo de CuboLab Healthcare.

---

## 📄 Licencia

Este proyecto es parte de la iniciativa CuboLab Healthcare.
