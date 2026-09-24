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

## Hacerlo más realista

El código deja preparado el cambio de materiales sin tocar C++:

1. En el editor: **Add → Add Feature or Content Pack → Starter Content** (o descarga materiales de
   **Quixel Megascans / Fab**, gratis con Unreal).
2. Crea un **Blueprint Class** hijo de `JJLevelBuilder` (por ejemplo `BP_Builder`) y en sus detalles
   rellena **Wall Materials** (índice = tipo de pared): 1 ladrillo, 2 metal, 3 piedra, 4 carne,
   10 panel técnico, 11 madera, 12 mármol. Asigna también **Floor Material** y **Ceiling Material**.
   Ejemplos del Starter Content: `M_Brick_Clay_New`, `M_Metal_Steel`, `M_Rock_Slate`, `M_Wood_Floor_Walnut_Polished`.
3. Crea un Blueprint hijo de `JJGameMode` (`BP_GameMode`), pon **Builder Class = BP_Builder** y selecciónalo en
   **Project Settings → Maps & Modes → Default GameMode**.
4. Para monstruos y armas realistas hay que sustituir las formas básicas por personajes con animaciones
   (por ejemplo de **Fab** o **Mixamo**); eso ya requiere trabajo en el editor.
