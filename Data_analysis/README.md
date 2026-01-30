# CuboLab - Interfaz de Análisis de Datos

## Descripción

Conjunto de aplicaciones desarrolladas con **Streamlit** que permiten analizar y visualizar datos relacionados con enfermeras, pacientes, residentes y los cubos empleados.

Las aplicaciones están diseñadas para facilitar la interacción y actualización de los datos almacenados en servidores externos. Los usuarios pueden seleccionar diferentes tablas, realizar análisis específicos y visualizar la información de manera clara y comprensible. Se incluyen predicciones de emociones para días futuros y análisis de texto de comentarios/intervenciones.

---

## Características Principales

- **Actualización de la base de datos**: botón para actualizar los datos con un solo clic directamente desde servidores externos.
- **Visualización de datos**: muestra tablas con información relevante (enfermeras, pacientes, personal, etc.) con múltiples columnas de análisis.
- **Interfaz intuitiva**: Desplegables y selectores para navegar fácilmente entre distintos análisis y tablas.
- **Análisis y predicción**: Opciones de análisis específicos con gráficos interactivos y predicciones de emociones usando modelos SARIMAX.
- **Análisis de texto**: Visualización mediante nubes de palabras según la emoción a partir de comentarios/intervenciones.
- **Gráficas interactivas**: Gráficas escalonadas con Plotly que permiten visualizar datos históricos y predicciones en tiempo real.

---

## Estructura del Proyecto

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

## Guía de Instalación y Ejecución

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

## Interfaz de Emociones (`interfaz_emociones_descritas.py`)

### ¿Qué hace?

Este script proporciona un análisis interactivo de las emociones registradas de usuarios/pacientes. Permite:

- **Seleccionar un usuario** de la lista disponible
- **Visualizar datos históricos** de emociones mediante gráficas escalonadas
- **Ver intervenciones/descripciones** asociadas a cada registro de emoción (al pasar el cursor)
- **Predecir emociones futuras** usando modelos SARIMAX para los próximos 1-30 días
- **Comparar gráficamente** datos históricos con predicciones
- **Descargar tabla de predicciones** para análisis posterior

### Dependencias Principales

- **streamlit**: framework para crear interfaces web interactivas.
- **pandas**: análisis y manipulación de datos.
- **plotly**: gráficas interactivas.
- **statsmodels**: modelos estadísticos avanzados (SARIMAX para predicciones).
- **requests**: descargar datos desde servidores externos.

### Fuente de Datos

Los datos se descargan directamente desde un servidor OSF (Open Science Framework) mediante URLs específicas configuradas en el script.

---

## Interfaz General (`interfaz_tablas_analisis.py`)

### ¿Qué hace?

Este script proporciona un análisis completo y flexible de múltiples tablas de datos en la base de datos de CuboLab. Permite:

- **Seleccionar y visualizar diferentes tablas**: Enfermeras, Pacientes, Centros, Intervenciones, Medidas, Sesiones, Sensores, etc.
- **Filtrar por Centro**: opción para filtrar todos los análisis por un centro específico.
- **Análisis estadísticos específicos**: cada tabla tiene análisis customizados (distribuciones, gráficos comparativos, evoluciones temporales).
- **Análisis de Intervenciones**: visualización de intervenciones por enfermera, paciente o tipo.
- **Análisis de Medidas**: estado emocional de pacientes, niveles de batería, evolución temporal.
- **Predicciones**: modelos ARIMA y Exponential Smoothing para predecir valores futuros.
- **Filtrado de datos**: actualización dinámica de datos locales con botón de refresh.

### Dependencias Principales

- **streamlit**: framework para crear interfaces web interactivas.
- **pandas**: análisis y manipulación de datos.
- **altair**: gráficas interactivas y visualizaciones.
- **statsmodels**: modelos estadísticos para predicciones (ARIMA, ExponentialSmoothing).

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

### Fuente de Datos

Los datos se cargan desde archivos CSV locales almacenados en la carpeta `/data`.

---

## Interfaz de Residentes (`interfaz_real_residentes.py`)

### ¿Qué hace?

Este script combina análisis de emociones con procesamiento avanzado de lenguaje natural (NLP) para analizar comentarios de residentes. Proporciona:

- **Análisis temporal de emociones**: gráficas escalonadas de emociones históricas por usuario.
- **Predicción de emociones futuras**: modelos SARIMAX para predecir emociones 1-30 días en el futuro.
- **Análisis de comentarios**: relación entre palabras y emociones mediante TF-IDF.
- **Visualización de patrones**: heatmaps mostrando palabras dominantes por emoción.
- **Nubes de palabras**: WordCloud generadas automáticamente por cada emoción.
- **Clustering de comentarios**: agrupación automática de comentarios similares mediante K-Means y PCA.
- **Descarga de resultados**: exportar análisis clustereados a CSV.

### Dependencias Principales

- **streamlit**: framework para interfaces web.
- **pandas & numpy**: manipulación de datos.
- **plotly**: gráficas interactivas.
- **statsmodels**: modelos SARIMAX para predicciones.
- **scikit-learn**: TF-IDF, PCA, K-Means clustering.
- **wordcloud**: generación de nubes de palabras.
- **matplotlib & seaborn**: visualizaciones estáticas (heatmaps, gráficos).

### Funcionalidades Principales

#### 1. Análisis Temporal
- Seleccionar usuario específico.
- Visualizar línea temporal de emociones.
- Ver comentarios asociados al pasar el cursor.

#### 2. Predicción SARIMAX
- Predicción configurable (1-30 días).
- Gráficas comparativas histórico + predicción.
- Tabla descargable con predicciones.

#### 3. Análisis de Texto
- **TF-IDF Vectorization**: extrae palabras clave por emoción.
- **Heatmap**: matriz de palabras vs emociones mostrando relevancia.
- **WordCloud**: visualización estética de palabras más frecuentes por emoción.
- **Clustering**: agrupa comentarios similares mediante K-Means (3 clusters).

### Preprocesamiento de Texto

El script realiza preprocesamiento automático:
- Conversión a minúsculas.
- Eliminación de caracteres especiales.
- Limpieza de espacios en blanco.
- Filtrado de comentarios vacíos.

### Fuente de Datos

Los datos se cargan desde un archivo CSV local:
- `df_real_descargado.csv`: Contiene datos de residentes con columnas `id_paciente`, `fecha`, `emocion` y `comentario`.
