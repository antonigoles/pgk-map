# Taski
- Prosty, płaski event system
- Upewnić się że particlesy sie nie zespuły przez brak tesktur (pewnie sie zepsuły)

# Backlog
- Wyciągnąć mapowanie mesh-obiekt z MeshRepository i wsadzić do GameObjectRepository albo zrobić na to osobny service

# Co zaimplementować do silnika w tej iteracji
- Jak przyśpieszyć liczenie przesuniętych Normali meshy w shaderze?
- Pullowanie eventów z drugiego wątku
- Odizolowanie handleowania wejścia z klawaiatury/myszy od glfw
- Proceduralnie generowane tekstury
- Zcentralizować fizyke -> transformacje i rotacje nie powinny być implementowane przez ręczne zmiany pozycji przy użyciu np. setPosition
- Kolizje do całkowitego przepisania, stare do wywalenia

# Co fajnie by było zaimplementować ale nie w tej iteracji
- Eventy -> przejście na event driven development po stronie Application Code'u

# Zrobione
- Zaimplementować AABB drzewa
- Zaimplementować tekstury dla GameObjectClusterów
- Developer UI
- Dodać wieloźródłowe oświetlenie
- Dodać typ oświetlenia: słońce
- Cacheowanie zoptymalizowanych Mesh'ów jako pliki .3dbin
- Ładowanie obiektów OBJ (działa jakoś 90% standardu)
- Blending
- Dodać profiler
- Soldniejszy podział architektury na Engine Code | Application Code
- Posprzątać w plikach
- Odizolowanie sterowania kamery od core elementów silnika
- Klasa Window rozbita na Application i Scene
- Podział komponentów silnika na drobniejsze grupy
- Rozbicie na wieloscenowość
- Wyciągnąć mapowanie shader-obiekty z ShaderRepository do GameObjectRepository albo zrobić na to osobny service
- Dodać funkcje na stepowanie logiki, fizyki, skryptów itd.
