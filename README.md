# ChiptuneVST

Un sintetizador chiptune hecho con JUCE, pensado para sacar sonidos de Game Boy sin tener que abrir un tracker cada vez que aparece una idea.

Tiene los cuatro sabores clásicos del hardware: dos pulsos, wave channel y ruido. No intenta ser una emulación religiosa; apunta más a ese punto entretenido donde el sonido es pequeño, filoso y musical antes de ponerse demasiado correcto.

## Qué trae

- Sintetizador MIDI con hasta 8 voces.
- Salidas **VST3**, **AU** y **Standalone**.
- Canales tipo Game Boy:
  - Pulse 1, con duty, sweep, PWM y vibrato.
  - Pulse 2, para leads y bajos más directos.
  - Wave, con una tabla editable de 32 pasos.
  - Noise, para percusiones y efectos.
- Envolvente ADSR.
- Arpegiador con patrones simples: octave, power, major, minor, maj7, dim y sus4.
- Bitcrush y saturación.
- Osciloscopio en la interfaz.
- Presets inspirados en LSDJ, Pokemon GB, FEZ / Disasterpeace, Trey Frey, Fake SID, hyperpop chip y drums de tracker.

## Compilar

Requisitos:

- macOS
- CMake 3.22 o superior
- Xcode / Command Line Tools
- Internet la primera vez, porque CMake descarga JUCE 8.0.4 con `FetchContent`

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Los artefactos quedan en:

```text
build/ChiptuneVST_artefacts/Release/
```

## Instalar en macOS

Hay un script para compilar, firmar localmente e instalar el VST3 y el AU en las carpetas de usuario:

```bash
./install.sh
```

Instala en:

```text
~/Library/Audio/Plug-Ins/VST3/ChiptuneVST.vst3
~/Library/Audio/Plug-Ins/Components/ChiptuneVST.component
```

Si usas Ableton Live, el script también intenta borrar la entrada vieja del plugin en la base local para que el rescan no se quede pegado con una versión anterior.

## Notas de uso

El plugin recibe MIDI y entrega audio estéreo. Para partir rápido, carga un preset y cambia el `Channel`:

- `Pulse 1` para leads con carácter y sweep.
- `Pulse 2` para líneas más estables.
- `Wave` para bajos redondos, pads chicos y sonidos dibujados a mano.
- `Noise` para hats, snares, clicks y efectos.

La tabla de onda se puede dibujar directamente en la interfaz. Es medio bruta a propósito: 32 valores, 4 bits, cero misterio.

## Estado

Proyecto personal, versión `1.0.0`. Funciona como instrumento, pero todavía hay espacio para pulir cosas como gestión de presets de usuario, automatización más fina y builds empaquetados.

## Licencia

Todavía no hay licencia definida. Si quieres usarlo en algo más serio que probarlo, abre un issue o escribe antes.

