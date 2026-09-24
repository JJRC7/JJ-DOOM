#include "JJLevels.h"

// Mapas en texto (los mismos que la versión web).
// Paredes: # ladrillo, M metal, S piedra, F carne, T panel técnico, W madera, G mármol,
//          X salida, D puerta, R puerta roja, U puerta azul
// Enemigos: z zombi, s sargento, i imp, d demonio, e espectro, l alma perdida, C cacodemonio, B barón
// Objetos: P jugador, h botiquín, t estimulante, v esfera de almas, a cargador, b cartuchos, q cohetes,
//          r armadura, g escopeta, c ametralladora, n lanzacohetes, k llave roja, u llave azul, o barril
const TArray<FJJLevelData>& JJLevels::Get()
{
	static TArray<FJJLevelData> Out;
	if (Out.Num() > 0)
	{
		return Out;
	}
		{
			FJJLevelData L;
			L.Name = TEXT("E1M1: EL HANGAR");
			L.Yaw = 0.0f;
			L.bBoss = false;
			L.FloorColor = FLinearColor(0.10f, 0.10f, 0.10f);
			L.CeilingColor = FLinearColor(0.12f, 0.12f, 0.13f);
			L.LightColor = FLinearColor(1.00f, 0.95f, 0.85f);
			L.Map = {
				TEXT("MMMMMMMMMMMMMMMMMMMMMMMMMMMM"),
				TEXT("M......M........M.........aM"),
				TEXT("M.P....M...z....M....z.....M"),
				TEXT("M......D........D..........M"),
				TEXT("M..a...M....o...M...o...i..M"),
				TEXT("M......M........M..........M"),
				TEXT("MMMDMMMMMMMMDMMMMMMMMMDMMMMM"),
				TEXT("#........#.........#.......#"),
				TEXT("#..z.....#....h....#...z...#"),
				TEXT("#........D.........D.......#"),
				TEXT("#...o.s..#....i....#...b...#"),
				TEXT("#........#.........#.......#"),
				TEXT("#....g...#####D#####...d...#"),
				TEXT("##D#######.........##D######"),
				TEXT("#.................i#.......S"),
				TEXT("#..i......z........#...k...S"),
				TEXT("#.........o....s...#.......S"),
				TEXT("#....h.........d...#...r...S"),
				TEXT("#..................#.......S"),
				TEXT("####R#######################"),
				TEXT("F.......i.......F.....s....F"),
				TEXT("F...h....o......D...i....h.F"),
				TEXT("F.......d.......F.........XF"),
				TEXT("FFFFFFFFFFFFFFFFFFFFFFFFFFFF"),
			};
			Out.Add(L);
		}
		{
			FJJLevelData L;
			L.Name = TEXT("E1M2: LAS ENTRA\u00D1AS");
			L.Yaw = 0.0f;
			L.bBoss = false;
			L.FloorColor = FLinearColor(0.14f, 0.05f, 0.03f);
			L.CeilingColor = FLinearColor(0.08f, 0.05f, 0.05f);
			L.LightColor = FLinearColor(1.00f, 0.55f, 0.35f);
			L.Map = {
				TEXT("SSSSSSSSSSSSSSSSSSSSSSSSSS"),
				TEXT("S.......S.........S......S"),
				TEXT("S..P....S....i....S..z...S"),
				TEXT("S.......D.........D......S"),
				TEXT("S...a...S....o....S...h..S"),
				TEXT("S.......S...l.....S......S"),
				TEXT("SSSSDSSSSSSSSDSSSSSSSDSSSS"),
				TEXT("F....s....F.......F......F"),
				TEXT("F...z.....F...d...F...i..F"),
				TEXT("F.........D.......D......F"),
				TEXT("F....o....F...c...F......F"),
				TEXT("F.........F.......F...b..F"),
				TEXT("FFFFFFDFFFFFFFFFFFFFFFFFFF"),
				TEXT("F.......e............F...F"),
				TEXT("F..i.......d.........F.r.F"),
				TEXT("F.......o.......i....F...F"),
				TEXT("F....h...............F...X"),
				TEXT("F..z.......k......z..F.a.F"),
				TEXT("F....................R...F"),
				TEXT("F...i.....o.......h..F.i.F"),
				TEXT("FFFFFFFFFFFFFFFFFFFFFFFFFF"),
			};
			Out.Add(L);
		}
		{
			FJJLevelData L;
			L.Name = TEXT("E1M3: LA REFINER\u00CDA");
			L.Yaw = 0.0f;
			L.bBoss = false;
			L.FloorColor = FLinearColor(0.18f, 0.17f, 0.15f);
			L.CeilingColor = FLinearColor(0.12f, 0.12f, 0.13f);
			L.LightColor = FLinearColor(0.85f, 0.95f, 1.00f);
			L.Sky.Add(FIntRect(1, 6, 21, 17));
			L.Map = {
				TEXT("TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT"),
				TEXT("T.......T..........T...........T"),
				TEXT("T..P....D..s....i..D..z.....h..T"),
				TEXT("T.......T..........T...........T"),
				TEXT("T..b....T....o.....T..q..s.....T"),
				TEXT("TTTDTTTTTTTTTTDTTTTTTTTTTTTDTTTT"),
				TEXT("#.....................#........W"),
				TEXT("#..z.....i......s.....#..z..s..W"),
				TEXT("#...SS.........SS.....#........W"),
				TEXT("#...SS...o.....SS..i..#.o....n.W"),
				TEXT("#.........l...........#........W"),
				TEXT("#..i.........z.....o..#WWWWDWWWW"),
				TEXT("#......d..............#........W"),
				TEXT("#...SS.........SS..z..#.i....i.W"),
				TEXT("#...SS..h.....aSS.....#........W"),
				TEXT("#.......s.............D..s.....W"),
				TEXT("#..o......i.......e...#...u....W"),
				TEXT("#.....................#........W"),
				TEXT("##########U###########WWWWWWWWWW"),
				TEXT("M......s..........i............M"),
				TEXT("M..h....o.......z.......d......M"),
				TEXT("M...........l..............b...M"),
				TEXT("M..............................X"),
				TEXT("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM"),
			};
			Out.Add(L);
		}
		{
			FJJLevelData L;
			L.Name = TEXT("E1M4: LA CATEDRAL");
			L.Yaw = -90.0f;
			L.bBoss = false;
			L.FloorColor = FLinearColor(0.06f, 0.14f, 0.10f);
			L.CeilingColor = FLinearColor(0.12f, 0.07f, 0.04f);
			L.LightColor = FLinearColor(1.00f, 0.85f, 0.60f);
			L.Sky.Add(FIntRect(23, 7, 28, 18));
			L.Map = {
				TEXT("GGGGGGGGGGGGGGXGGGGGGGGGGGGGGG"),
				TEXT("G......i.......i......i......G"),
				TEXT("G..C......................C..G"),
				TEXT("G............................G"),
				TEXT("G....h..............h........G"),
				TEXT("G......d.........d...........G"),
				TEXT("GGGGGGGGGGGGGGRGGGGGGGGGGGGGGG"),
				TEXT("G......G..............W......W"),
				TEXT("G.s..l.G..GG......GG..W..z...W"),
				TEXT("G......G..GG..s...GG..W......W"),
				TEXT("G..k...G......C.......W.o..l.W"),
				TEXT("G......G.i..........i.W......W"),
				TEXT("G.i..e.U......b.......D......W"),
				TEXT("G......G..GG..i...GG..W..u...W"),
				TEXT("G..h...G..GG......GG..W......W"),
				TEXT("G......G..z...o....z..W.l..s.W"),
				TEXT("G.l..s.G......d.......W......W"),
				TEXT("G......G.s..........s.W..o...W"),
				TEXT("G......G.....q..h.....W......W"),
				TEXT("GGGGGGGGGGGGGGDGGGGGGGGGGGGGGG"),
				TEXT("G............................G"),
				TEXT("G..o....z..........z....o....G"),
				TEXT("G............................G"),
				TEXT("G.....a......P.......a.......G"),
				TEXT("G..h......................h..G"),
				TEXT("G............................G"),
				TEXT("GGGGGGGGGGGGGGGGGGGGGGGGGGGGGG"),
			};
			Out.Add(L);
		}
		{
			FJJLevelData L;
			L.Name = TEXT("E1M5: EL TRONO DE JJ");
			L.Yaw = -90.0f;
			L.bBoss = true;
			L.FloorColor = FLinearColor(0.14f, 0.05f, 0.03f);
			L.CeilingColor = FLinearColor(0.08f, 0.05f, 0.05f);
			L.LightColor = FLinearColor(1.00f, 0.40f, 0.25f);
			L.Sky.Add(FIntRect(1, 1, 24, 16));
			L.Map = {
				TEXT("FFFFFFFFFFFFXFFFFFFFFFFFFF"),
				TEXT("F..........i.....i.......F"),
				TEXT("F...C................C...F"),
				TEXT("F........................F"),
				TEXT("F..FF......B.........FF..F"),
				TEXT("F..FF................FF..F"),
				TEXT("F........................F"),
				TEXT("F.i....l..........l....i.F"),
				TEXT("F........................F"),
				TEXT("F..FF.......o........FF..F"),
				TEXT("F..FF................FF..F"),
				TEXT("F....d..............d....F"),
				TEXT("F........................F"),
				TEXT("F.h....s..........s....h.F"),
				TEXT("F........................F"),
				TEXT("F...q......n.....v...q...F"),
				TEXT("F........................F"),
				TEXT("FFFFFFFFFFFFDFFFFFFFFFFFFF"),
				TEXT("F..a..................a..F"),
				TEXT("F........................F"),
				TEXT("F...b........P.......b...F"),
				TEXT("F..r..................h..F"),
				TEXT("F........................F"),
				TEXT("FFFFFFFFFFFFFFFFFFFFFFFFFF"),
			};
			Out.Add(L);
		}
	return Out;
}
