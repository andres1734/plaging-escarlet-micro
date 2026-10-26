# Scarlett Warmth VST3 - Guía para Visual Studio 2022 & FL Studio

Esta guía te explica cómo abrir el proyecto en **Visual Studio 2022 (el moradito)**, compilarlo en 64 bits y tenerlo sonando en **FL Studio**.

---

## 🛠️ Requisitos Previos en tu PC
1. **Visual Studio 2022** (Community, Professional o Enterprise).
   - Asegúrate de tener marcada la carga de trabajo: **"Desarrollo para el escritorio con C++"** (Desktop development with C++).
2. **CMake** (versión 3.22 o superior).
   - Descarga gratuita: [https://cmake.org/download/](https://cmake.org/download/)
   - Durante la instalación, marca la casilla **"Add CMake to system PATH"**.

---

## 🚀 Método de 1 Clic (Recomendado)

1. **Descomprime** esta carpeta en el lugar que prefieras (por ejemplo, en tus Documentos o Escritorio).
2. Haz doble clic en el archivo:
   ```cmd
   Abrir_en_VisualStudio_2022.bat
   ```
   - Este script detectará tu CMake y creará la solución nativa `ScarlettWarmth.sln` dentro de la carpeta `build`.
   - Se abrirá **Visual Studio 2022** con el proyecto cargado automáticamente.

3. **Dentro de Visual Studio 2022**:
   - En la barra de herramientas superior, cambia **Debug** por **Release**.
   - Asegúrate de que la arquitectura sea **x64**.
   - Presiona las teclas: `Ctrl + Shift + B` (o en el menú superior: **Compilar > Compilar solución**).

4. **¡Listo! Despliegue automático a FL Studio**:
   - El proyecto incluye un comando post-compilación que copia el archivo `Scarlett Warmth Preamp.vst3` directamente a:
     ```text
     C:\Program Files\Common Files\VST3\Scarlett Warmth Preamp.vst3
     ```
   - Si Windows te pide permisos de administrador para la carpeta Program Files, simplemente haz clic derecho en:
     ```cmd
     Instalar_en_FL_Studio.bat
     ```
     y selecciona **"Ejecutar como administrador"**.

---

## 🎹 Cómo abrirlo en FL Studio (20 / 21 / 24)

1. Abre **FL Studio**.
2. En la barra superior, ve a **Options > Manage plugins**.
3. En la esquina superior izquierda de la ventana de plugins, haz clic en el botón naranja **"Find installed plugins"**.
4. FL Studio escaneará tu carpeta de VST3 y verás aparecer en color naranja o resaltado:
   **"Scarlett Warmth Preamp"** (Tipo: VST3, 64-bit Effect).
5. Abre el **Mixer de FL Studio (F9)**:
   - Selecciona el canal donde grabas tu micrófono Focusrite Scarlett CM25.
   - En los slots de inserción (Slot 1 o 2), haz clic y selecciona **Scarlett Warmth Preamp**.
6. ¡Listo! Ya tienes la saturación analógica a tubos, el control de aspereza del micrófono CM25 y la calidez en tu pista vocal.

---

## 🎛️ Parámetros del Plugin
- **Drive / Input**: Nivel de ganancia y saturación armónica (Tubos 610 / Neve 1073).
- **Chest Warmth (220Hz)**: Calidez y cuerpo pectoral para voces delgadas.
- **CM25 Anti-Stridence**: Atenuador quirúrgico de la resonancia aguda (5.8 kHz) de la cápsula Scarlett CM25.
- **Transformer Iron**: Saturación magnética de transformador (3º armónico denso).
- **Silk Air (12kHz)**: Brillo aterciopelado sin sibilancia.
- **HPF (Filtro Paso Alto)**: Flat, 40 Hz, 80 Hz, 120 Hz.
- **Output Trim**: Compensación de volumen de salida (-12dB a +12dB).
- **Bypass**: Comparación A/B directa sin latencia.
