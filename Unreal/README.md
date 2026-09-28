# JJ-DOOM para Unreal Engine 5

Versión en C++ de Infierno JJ para Unreal Engine 5 (probada en papel para 5.3, 5.4 y 5.5).
Los 5 niveles se generan por código a partir de los mismos mapas de texto que la versión web,
así que **no hace falta importar ningún modelo**: todo usa las formas básicas del motor.

Lo que aporta Unreal frente a la versión web:

- Iluminación global en tiempo real (**Lumen**) y sombras dinámicas de cada lámpara.
- Linterna (tecla **F**) y fogonazos que iluminan las paredes.
- Niebla volumétrica, cielo con atmósfera y sol de atardecer en las zonas al aire libre.
- Mirar arriba y abajo, correr, puertas que suben al techo, proyectiles con luz propia y explosiones.

> **Importante:** este código no se ha podido compilar ni probar desde aquí (no hay Unreal Engine en
> el entorno donde se escribió). Es probable que al compilar aparezca algún error pequeño; si pasa,
> copia el mensaje de error y se corrige.

## Requisitos (Windows)

1. **Unreal Engine 5.4** (o 5.3 / 5.5) desde el Epic Games Launcher.
2. **Visual Studio 2022** con la carga de trabajo **"Desarrollo de juegos con C++"**
   (incluye el SDK de Windows y las herramientas de Unreal).

## Cómo abrirlo

1. Descarga o clona este repositorio.
2. Entra en `Unreal/JJDoom/`, haz clic derecho sobre **`JJDoom.uproject`** y elige
   **"Generate Visual Studio project files"**.
   - Si tienes otra versión de Unreal, antes elige **"Switch Unreal Engine version..."**.
3. Haz doble clic en `JJDoom.uproject`. Cuando pregunte si quieres compilar los módulos, di **Sí**.
   (También puedes abrir `JJDoom.sln` y compilar **Development Editor | Win64**).
4. En el editor pulsa **Play**. El nivel se construye solo al empezar.

## Controles

| Tecla | Acción |
|---|---|
| WASD / flechas | Moverse / girar |
| Ratón | Apuntar |
| Clic / Espacio | Disparar |
| E | Abrir puertas / usar la salida |
| Shift | Correr |
| F | Linterna |
| 1-4 / rueda | Cambiar de arma |
| Enter | Continuar tras terminar el nivel o al morir |
| + / − (o Re Pág / Av Pág) | Subir / bajar el brillo |

También funciona con mando (Xbox/PlayStation).

## Estructura del código (`Source/JJDoom`)

| Archivo | Qué hace |
|---|---|
| `JJLevels.cpp` | Los 5 mapas en texto (mismo formato que la versión web). |
| `JJLevelBuilder` | Construye paredes, suelo, techo, lámparas, puertas, enemigos y objetos; calcula el camino de los enemigos. |
| `JJCharacter` | Jugador: movimiento, 4 armas, linterna, daño, recoger objetos. |
| `JJEnemy` | 8 tipos de enemigo con IA (perseguir, disparar, embestir, volar). |
| `JJTypes.cpp` | Estadísticas de enemigos y armas (vida, daño, velocidad...). |
| `JJGameMode` | Niveles, salida, muerte, victoria y estadísticas. |
| `JJHUD` | Interfaz: salud, armadura, munición, llaves, mensajes. |
| `JJDoor`, `JJExitSwitch`, `JJPickup`, `JJBarrel`, `JJProjectile`, `JJFlash` | Puertas, salida, objetos, barriles, proyectiles y destellos. |

## Hacerlo más realista (sin programar)

El juego detecta solo estos paquetes gratuitos de Unreal y los usa si están en el proyecto:

| Paquete | Qué cambia |
|---|---|
| **Starter Content** | Paredes, suelos, techos, puertas, zócalos y vigas con materiales reales: ladrillo, metal, piedra, madera, mármol, hormigón, baldosas... |
| **Third Person** | Zombis, sargentos, imps y el Barón pasan a ser personajes 3D **animados** (caminan y corren) y caen como **muñeco de trapo** al morir. |

Cómo añadirlos (una sola vez):

1. Abre el proyecto en el editor de Unreal.
2. Abajo, abre el **Content Drawer** (Ctrl + Espacio) y pulsa **+ Add** (o **Añadir**).
3. Elige **Add Feature or Content Pack...**
4. En la pestaña **Content**, elige **Starter Content** → **Add to Project**.
5. Vuelve a abrir **Add Feature or Content Pack...**, pestaña **Blueprint**, elige **Third Person** → **Add to Project**.
6. Pulsa **Play**. No hace falta tocar nada más.

Además, siempre (con o sin paquetes) el juego usa niebla volumétrica con haces de luz, resplandor,
viñeta, grano de película, lámparas que parpadean, zócalos, molduras y vigas en el techo.

### Usar tus propios materiales

1. Crea un **Blueprint Class** hijo de `JJLevelBuilder` (por ejemplo `BP_Builder`) y rellena **Wall Materials**
   (índice = tipo de pared): 1 ladrillo, 2 metal, 3 piedra, 4 carne, 10 panel técnico, 11 madera, 12 mármol.
   También **Floor Material** y **Ceiling Material**. Tienen prioridad sobre el Starter Content.
2. Crea un Blueprint hijo de `JJGameMode` (`BP_GameMode`), pon **Builder Class = BP_Builder** y selecciónalo en
   **Project Settings → Maps & Modes → Default GameMode**.
3. Materiales gratuitos de alta calidad: **Fab** (incluye Quixel Megascans), desde el propio editor.

## Monstruos y armas reales (modelos de Fab)

El juego puede usar cualquier personaje 3D con animaciones y cualquier modelo de arma, sin programar:

1. **Descarga los modelos**: en el editor pulsa el botón **Fab** (arriba del Content Drawer), busca
   monstruos o armas (filtra por **Free** para los gratuitos; por ejemplo los personajes *Paragon* de Epic)
   y pulsa **Add to Project**.
2. **Crea el Blueprint del modo de juego**: en el Content Drawer, **+ Añadir → Clase Blueprint**, busca
   **JJGameMode** en "Todas las clases" y llámalo `BP_GameMode`.
3. Ábrelo y en el panel **Detalles**, sección **JJ | Modelos**:
   - **Monster Visuals**: pulsa **+**, elige el tipo de enemigo (Zombie, Imp, Demon, Caco, Baron...) y asigna:
     - **Mesh**: la malla con esqueleto del monstruo (icono rosa, *Skeletal Mesh*).
     - **Anim Blueprint**: su Animation Blueprint si el paquete trae uno, **o** bien
       **Walk Animation** / **Attack Animation** / **Death Animation** con animaciones sueltas.
     - **Scale**, **Yaw Offset** (giro; normalmente -90) y **Height Offset** si queda grande, girado o hundido.
   - **Weapon Visuals**: pulsa **+**, elige el arma (Pistol, Shotgun, Chaingun, Rocket) y asigna
     **Static Mesh** o **Skeletal Mesh**, y ajusta **Offset**, **Rotation** y **Scale** hasta que se vea bien en pantalla.
4. **Compila y guarda** el Blueprint, y en **Editar → Configuración del proyecto → Mapas y modos**
   pon **Default GameMode = BP_GameMode**.
5. Pulsa **Play**. Los tipos que no tengan modelo asignado siguen usando el maniquí o las formas básicas.
