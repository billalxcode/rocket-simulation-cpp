Sempurna! Saya sudah punya gambaran lengkap. Berikut rekomendasi arsitektur lengkapnya:

---

## Arsitektur Rekomendasi: Professional Rocket Simulation Engine

### Filosofi Desain
> **Separation of Concerns** — setiap layer tidak boleh tahu detail implementasi layer lain. Physics core tidak peduli soal GUI, GUI tidak peduli soal numerik integrator.

---

## Struktur Modul (Top-Level)

```
rocket-sim/
├── core/           → Physics engine murni, no dependencies eksternal
├── models/         → Model fisika spesifik (aero, propulsi, gravitasi)
├── vehicle/        → Definisi struktur roket & stage
├── simulation/     → Orchestrator, integrator numerik, event system
├── environment/    → Atmosfer, planet, ephemeris
├── io/             → Parser JSON/YAML, serializer hasil
├── api/            → Python binding (pybind11)
├── gui/            → Visualisasi 3D/2D, plotting
├── cli/            → Entry point headless
└── tests/          → Unit & integrasi per modul
```

---

## Detail Setiap Modul

---

### 1. `core/` — Physics Primitives
**Tanggung jawab:** Tipe data fundamental dan matematika vektor/matriks. Tidak boleh ada dependency ke modul lain.

```
core/
├── math/
│   ├── Vector3.hpp        → Vec3 operasi dasar
│   ├── Quaternion.hpp     → Rotasi attitude
│   ├── Matrix3x3.hpp      → Inertia tensor
│   └── CoordTransform.hpp → ECI ↔ ECEF ↔ NED ↔ Body frame
├── types/
│   ├── StateVector.hpp    → [pos, vel, mass, attitude] — state utama simulasi
│   ├── ForceVector.hpp    → Agregasi semua gaya
│   └── PhysicalConstants.hpp → G, R_earth, atm constants
└── interfaces/
    ├── IForceModel.hpp    → Interface semua model gaya
    ├── IIntegrator.hpp    → Interface integrator numerik
    └── IEnvironment.hpp   → Interface environment
```

**Catatan penting:** `StateVector` adalah struct yang dibawa oleh seluruh pipeline simulasi — desain ini dengan sangat hati-hati karena semua modul bergantung padanya.

---

### 2. `models/` — Physics Models
**Tanggung jawab:** Implementasi konkret tiap model fisika. Semua implement `IForceModel` dari `core/interfaces/`.

```
models/
├── aerodynamics/
│   ├── AtmosphericDrag.hpp      → F_drag = ½ρv²CdA
│   ├── LiftModel.hpp            → Fin lift, angle of attack
│   ├── MachRegimeSelector.hpp   → Subsonic / transonic / supersonic / hypersonic
│   └── AeroCoeffTable.hpp       → Lookup table Cd vs Mach (interpolasi)
│
├── propulsion/
│   ├── ThrustCurve.hpp          → Thrust(t) dari data engine
│   ├── PropellantMassFlow.hpp   → dm/dt = thrust / (Isp * g0)
│   ├── NozzleExitPressure.hpp   → Thrust koreksi tekanan ambient
│   └── EngineRegistry.hpp       → Database engine (YAML-driven)
│
└── gravity/
    ├── PointMassGravity.hpp     → F = GMm/r²  (simple)
    ├── J2PerturbationModel.hpp  → Oblateness Bumi
    ├── NBodyGravity.hpp         → Multi-body (Bumi + Bulan + Matahari)
    └── EphemerisProvider.hpp    → Posisi planet dari SPICE / tabel
```

**Catatan:** `MachRegimeSelector` penting karena Cd berubah drastis di sekitar Mach 1 — simulasi akan tidak akurat tanpa ini.

---

### 3. `vehicle/` — Rocket Definition
**Tanggung jawab:** Representasi struktur roket, stage, dan komponen. Di-load dari file YAML.

```
vehicle/
├── Stage.hpp              → Satu stage: dry mass, propellant mass, engine list
├── RocketVehicle.hpp      → Kumpulan stages + urutan separasi
├── MassProperties.hpp     → CG, inertia tensor berubah seiring bahan bakar habis
├── StageSeparationEvent.hpp → Trigger: kapan stage dilepas (altitude / time / burnout)
├── FinGeometry.hpp        → Dimensi fin untuk model aero
└── PayloadDefinition.hpp  → Payload di ujung roket
```

**Pola desain:** `RocketVehicle` adalah **tree of stages**. Saat separation event terjadi, stage dibuang dan simulasi melanjutkan dengan vehicle yang lebih ringan.

---

### 4. `environment/` — External Environment
**Tanggung jawab:** Semua kondisi eksternal yang mempengaruhi roket.

```
environment/
├── atmosphere/
│   ├── IAtmosphereModel.hpp      → Interface
│   ├── ISA1976.hpp               → International Standard Atmosphere
│   ├── NRLMSISE00.hpp            → Model atmosfer realistis (high altitude)
│   └── AtmosphereFactory.hpp     → Pilih model berdasarkan config
│
├── planet/
│   ├── PlanetModel.hpp           → Radius, GM, rotasi
│   ├── EarthModel.hpp
│   └── MarsModel.hpp
│
└── wind/
    ├── IWindModel.hpp
    ├── ConstantWind.hpp          → Untuk testing
    └── WindProfileTable.hpp      → Wind vs altitude dari data
```

---

### 5. `simulation/` — Orchestrator & Integrator
**Tanggung jawab:** Jantung simulasi. Mengkoordinasikan semua model, menjalankan loop integrasi, menangani event.

```
simulation/
├── integrators/
│   ├── IIntegrator.hpp
│   ├── RK4Integrator.hpp         → Fixed step, sederhana
│   ├── RK45Integrator.hpp        → Adaptive step (rekomendasi default)
│   └── IntegratorFactory.hpp     → Pilih integrator dari config
│
├── SimulationEngine.hpp          → Main loop: for each step → gather forces → integrate → check events
├── SimulationConfig.hpp          → dt, t_max, toleransi, pilihan model
├── EventSystem.hpp               → Event queue: apogee, burnout, separation, ground impact
├── EventDetector.hpp             → Zero-crossing detection untuk events
├── SimulationState.hpp           → Snapshot state di setiap timestep
└── Telemetry.hpp                 → Buffer hasil simulasi (time series)
```

**Rekomendasi integrator:** Gunakan **RK45 adaptive step** sebagai default. Fixed RK4 boleh tersedia tapi bukan default — roket saat liftoff butuh step kecil, saat coasting di orbit bisa step besar. Adaptive step otomatis menangani ini.

**EventSystem** sangat penting untuk multi-stage — deteksi `burnout` memicu `StageSeparation`, lalu `vehicle` di-update.

---

### 6. `io/` — Data Layer
**Tanggung jawab:** Semua baca/tulis file. Modul lain tidak boleh langsung baca file.

```
io/
├── parsers/
│   ├── VehicleParser.hpp         → YAML → RocketVehicle
│   ├── SimConfigParser.hpp       → YAML → SimulationConfig
│   └── EngineDatabaseParser.hpp  → YAML → EngineRegistry
│
├── exporters/
│   ├── TelemetryExporter.hpp     → SimulationState[] → JSON / CSV
│   ├── ReportGenerator.hpp       → Results → PDF (via LaTeX atau HTML→PDF)
│   └── KMLExporter.hpp           → Trajectory → Google Earth KML
│
└── schema/
    ├── vehicle.schema.json        → Validasi file input roket
    └── simconfig.schema.json
```

---

### 7. `api/` — Python Binding
**Tanggung jawab:** Expose C++ core ke Python untuk scripting & automation.

```
api/
├── python/
│   ├── bindings.cpp              → pybind11 entry point
│   ├── RocketSimPy.hpp           → Wrapper tipis agar Python-friendly
│   └── CMakeLists.txt
│
└── rocketsim/                    → Python package
    ├── __init__.py
    ├── vehicle.py                → Helper builder untuk RocketVehicle
    ├── run.py                    → rocketsim.run(config) → results
    └── plot.py                   → matplotlib helper dari hasil simulasi
```

**Contoh penggunaan API:**
```python
import rocketsim
result = rocketsim.run("my_rocket.yaml", "config.yaml")
rocketsim.plot.trajectory(result)
```

---

### 8. `gui/` — Visualisasi
**Tanggung jawab:** Real-time 3D/2D display. Harus bisa jalan **tanpa** mengganggu simulation loop.

```
gui/
├── renderer/
│   ├── SceneRenderer.hpp         → OpenGL / Vulkan scene
│   ├── RocketMeshLoader.hpp      → Load model 3D roket
│   ├── TrajectoryRenderer.hpp    → Gambar jalur roket di 3D globe
│   └── AtmosphereShader.hpp      → Visual layer atmosfer
│
├── widgets/
│   ├── TelemetryPanel.hpp        → Altitude, velocity, acceleration live
│   ├── StageIndicator.hpp        → Status tiap stage
│   ├── PlotWidget.hpp            → Real-time graph (altitude vs time, dll)
│   └── ControlPanel.hpp          → Play / pause / speed up simulasi
│
├── GUIApplication.hpp            → Main window (Qt atau Dear ImGui)
└── SimulationBridge.hpp          → Thread-safe bridge antara sim loop ↔ GUI
```

**Rekomendasi framework GUI:** **Qt6** untuk tools engineering profesional (mature, cross-platform), atau **Dear ImGui** jika ingin lebih ringan dan embedded-friendly.

**`SimulationBridge`** wajib ada — simulation loop berjalan di thread terpisah, GUI di main thread. Bridge ini pakai lock-free ring buffer agar tidak ada frame drop.

---

## Dependency Graph Antar Modul

```
gui ──────────────────────────────────┐
api (python) ────────────────────┐    │
cli ─────────────────────────┐   │    │
                             ↓   ↓    ↓
                        [ simulation ]
                         ↓    ↓    ↓
                    [models] [vehicle] [environment]
                         ↓    ↓    ↓
                           [ io ]
                              ↓
                           [ core ]
                         (no deps)
```

**Aturan keras:** Panah hanya boleh ke bawah. `core` tidak boleh import dari `models`. `models` tidak boleh import dari `simulation`. Pelanggaran aturan ini = technical debt.

---

## Rekomendasi Build System

```
CMakeLists.txt (root)
├── core          → static library
├── models        → static library (links core)
├── vehicle       → static library (links core)
├── environment   → static library (links core, models)
├── simulation    → static library (links semua di atas)
├── io            → static library (links simulation)
├── api           → shared library .so / .pyd (pybind11)
├── gui           → executable (links io, simulation)
└── cli           → executable (links io, simulation)
```

Setiap modul dikompilasi sebagai **static library** terpisah sehingga unit test bisa link hanya modul yang dibutuhkan tanpa menarik seluruh codebase.

---

## Ringkasan Pemisahan

| Modul | Boleh tahu tentang | Tidak boleh tahu tentang |
|---|---|---|
| `core` | Matematika & tipe data saja | Semua modul lain |
| `models` | `core` | `simulation`, `gui`, `io` |
| `vehicle` | `core` | `models`, `simulation` |
| `environment` | `core`, `models` | `simulation`, `gui` |
| `simulation` | Semua di atas | `gui`, `io`, `api` |
| `io` | `simulation`, `vehicle` | `gui`, `api` |
| `gui` | `simulation`, `io` | `api` |
| `api` | `simulation`, `io` | `gui` |

---

Dengan arsitektur ini, kamu bisa mulai dari `core` → `models` → `simulation` dulu sebagai MVP, lalu tambahkan `gui` dan `api` belakangan tanpa refactor besar. Mau saya bantu mulai implementasi modul tertentu?