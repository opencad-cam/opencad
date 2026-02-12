# OpenCAD

**Modüler CAD/CAE Platform** - Mühendislik odaklı açık kaynak CAD yazılımı.

## 🚀 Özellikler

- **Geometri Kernel**: OpenCASCADE Technology (B-Rep, NURBS)
- **Primitifler**: Box, Cylinder, Sphere, Cone, Torus
- **Boolean Operasyonlar**: Fuse, Cut, Common
- **Dönüşümler**: Translate, Rotate, Scale, Mirror
- **Dosya Formatları**: STEP (import/export), STL (export)
- **3D Viewport**: OpenGL tabanlı, kolay navigasyon
- **Modern UI**: Qt6, Dark theme

## 📋 Gereksinimler

- CMake 3.20+
- C++17 uyumlu derleyici
- OpenCASCADE 7.6+
- Qt6 (Core, Widgets, OpenGL, OpenGLWidgets)
- OpenGL 3.3+

## 🔧 Derleme

### Windows (Visual Studio)

```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### Linux

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

## 📁 Proje Yapısı

```
opencad/
├── src/
│   ├── core/           # Geometri çekirdeği
│   │   └── geometry/   # Shape, Primitives, BooleanOps, Transform
│   ├── io/             # Dosya I/O
│   │   ├── step/       # STEP handler
│   │   └── mesh/       # STL handler
│   ├── ui/             # Kullanıcı arayüzü
│   │   └── viewport/   # 3D görüntüleme
│   └── app/            # Ana uygulama
├── tests/              # Unit testler
├── docs/               # Dokümantasyon
└── CMakeLists.txt      # Ana CMake dosyası
```

## 🤖 AI & Otomasyon (YENİ)
 
OpenCAD artık **Strict Domain Parser** profili ile güçlendirilmiş bir yapay zeka asistanına sahiptir.
 
### Özellikler
- **Model**: `opencad-parser` (Qwen2.5-Coder-7B-Instruct tabanlı).
- **Protokol**: Deterministik JSON formatı.
- **Güvenilirlik**: Hallucination (uydurma) minimize edildi, belirsiz durumlarda "ambiguous" döner.
- **Python Bridge**: JSON çıktısını doğrudan CadQuery scriptlerine dönüştürür.
 
### Desteklenen Komutlar
Doğal dil ile aşağıdaki şekilleri oluşturabilirsiniz:
- **Kutu/Küp**: "10x10x10 bir kutu yap"
- **Silindir**: "Yarıçapı 5, yüksekliği 20 silindir"
- **Küre**: "Çapı 10 olan bir küre"
- **Boru/Tüp/Flüt**: "Dış çap 10, iç çap 8, boy 50 boru"
- **Koni**: "Taban 10, tepe 0, boy 20 koni"
 
### Kurulum
AI özelliklerini aktif etmek için Ollama gereklidir:
```bash
cd scripts/cadquery
ollama create opencad-parser -f Modelfile
```
 
## 🗺️ Yol Haritası

- [x] **Faz 1**: Minimum Viable CAD (temel primitifler, boolean, viewport)
- [ ] **Faz 2**: Parametrik Sketch (2D çizim, constraints)
- [ ] **Faz 3**: Feature-Based Modeling (history tree, parametrik)
- [ ] **Faz 4**: FEM/CFD Entegrasyonu (ElmerFEM, OpenFOAM)
- [ ] **Faz 5**: Assembly Sistemi
- [ ] **Faz 6**: Performans Optimizasyonu

## 📄 Lisans

GPL-3.0

## 🤝 Katkıda Bulunma

Pull request'ler memnuniyetle karşılanır!
