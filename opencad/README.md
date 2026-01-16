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
