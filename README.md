O‘YINNING QISQA TA’RIFI (High-level)

ThreadWalker — bu “sabab — natija” (causality) aloqalarini ko‘rish, kesish va yangi bog‘lash orqali dunyoni qayta yozadigan puzzle–action o‘yin. Har bir obyekt, hodisa va hissiyot “Thread Node” sifatida mavjud: o‘yinchi Thread Sight yordamida ularni ko‘radi, kesadi yoki yangi bog‘laydi va shunday qilib atrof-muhit, NPC va mexanizmlarning xatti-harakatini o‘zgartiradi.

O‘YIN MEXANIKASI — NIMA VA QANDAY ISHLAYDI

Thread Layer (ko‘rinmas qatlam)

O‘yinchi special rejimga (Thread Sight) o‘tadi: dunyo sekinlashadi va barcha threadlar neon chiziqlar bilan ko‘rinadi.

Har bir chiziq: manba (source node) → maqsad (target node), signal turi (physical, emotion, time, memory) va kuch (strength).

Asosiy harakatlar

See — Thread Sightni yoqish (Shift yoki V). Chiziqlar, node’lar va ularning turini ko‘rish.

Analyze — Node ustiga kursorni olib borish: tooltip (node turi, input/output, current state).

Cut — Raycast yoki gest bilan chiziqni kesish (masalan, sakkizli motion yoki tugma) — bog‘lanish o‘chirildi.

Weave (Link) — Node A → Node B ni bog‘lash (dynamically create thread). Shartlar va signal turi tanlanadi.

Activate — o‘zgartirishni tasdiqlash va dunyoning dinamik reaksiyasini kuzatish.

Thread turlari (masalan)

Physical: kuch, shamol, tutun, o‘simlik o‘sishi.

Emotion: qo‘rqish, umid, g‘azab — NPC xulqini o‘zgartiradi.

Time: sekinlashish, rewind, tezlashtirish — hudud yoki ob’ektlarga ta’sir.

Memory: NPC es-hayotini o‘zgartiradi (oldingi holatni tiklash yoki o‘chirish).

Causality Simulation

Har bir thread o‘zgartirilganda engine chain reaction hisoblashi kerak: bir node o‘zgarishi → target nodeda hodisa chaqiriladi → keyingi threadlar tarqaladi. Bu real-time va deterministic bo‘lishi zarur (tuzilgan qoidalar asosida).

O‘YIN LOOP (PLAYER EXPERIENCE)

Kirish: o‘yinchi hududga kiradi, maqsad beriladi (masalan, ko‘prikni o‘tkazish yoki NPCni tinchlantirish).

Kuzatish: Thread Sightni yoqib, mavjud bog‘lanishlarni tahlil qiladi.

Eksperiment: bir nechta thread kesadi yoki bog‘laydi.

Natija: dunyo reaktsiya qiladi — yangi yo‘l ochiladi yoki yangi muammo paydo bo‘ladi.

Yechim: o‘yinchi optimal rewiring topadi va maqsadga erishadi.

Progression: yangi ability (ko‘proq thread turi, kuch), yangi zonalar ochiladi.

MISOLLAR — ANIQ SITUATSIYALAR

Broken bridge: Bridge unstable ← Wind. YeChimlar: Wind→Bridge kesib, bridge barqarorlashadi; yoki Plant→Bridge bog‘lab, plant o‘sib ko‘prikni to‘ldiradi; yoki Wind→Enemy qilib, dushmanni ko‘prikdan olib tashlash.

Guarded door: Door opens ← Enemy alarm. YeChim: Enemy alarm thread’ini Memory→Forget boylash yoki Calm→Enemy bog‘lab, u tunashadi va o‘tib ketish mumkin bo‘ladi.

Time puzzle: Moving platforms are synced by Time threads. Weave to change speeds/phase to cross.

LEVEL-DESIGN & PROGRESSION

Act structure: Har act yangi thread turlarini tanishtiradi (Act 1: physical; Act 2: emotion; Act 3: time & memory kombinatsiyalari).

Sandbox zones: Early levels — kichik, ko‘proq eksperiment; keyin — kombinatsion jumboqlar.

Emergent puzzles: Har bir darajadagi ob’ektlar bilan bir nechta hal yo‘llar bo‘ladi — hakamlar bunday innovatsion emergent yechimlarni qadrlaydi.

AI & NPC XULQI

NPC’lar thread qiymatiga ko‘ra holat o‘zgartiradi: qo‘rqish → qochish, g‘azab → hujum, xotira o‘chirildi → yo‘qolib ketish.

AI state machine + thread hooks: thread o‘zgarganda AI state update bo‘ladi.

Panic behavior & flocking mumkin — emergent scenariolar.

KONTROLS & UI (FOCUS ON CLARITY)

Mouse/Keyboard: standard FPS/3rd-person controls.

ThreadSight toggle (hold): world slow-motion + overlay.

Selection cursor: highlight node with info panel (left click select node, right click select second node → press Link).

Quick-action wheel: cut, link, set-signal-type (for advanced players).

Tutorial prompts: kontekst asosida (birinchi marta link qilishda mini-guides).

ART & AUDIO DIRECTION

Visual: Minimalistic sci-fantasy. Physical world — muted, matte low-poly; Thread layer — neon colors, pulsing, smooth bezier lines. Contrast juda muhim.

VFX: Thread cut — particle burst; weave — soft glow and pulse along new thread; world reaction — subtle mesh animation (plant grows, bridge vibrates).

Sound: Each thread turi uchun audio signature: physical — metallic creak; emotion — distant choir; time — reversed chime. UX audio: cut-snap, weave-hum.

Music: Ambient score that swells during big causal shifts.

TECHNICAL TARGETS & PERFORMANCE

Target framerate: 60 FPS on mid-range GPU; scalable to 120+ for high-end.

Deterministic causality: simulation must be stable across runs (important for prototyping & debugging).

Debugging tools: Thread inspector, step-simulation (tick-by-tick), history/undo for player dev/testing.

Save system: store scene graph + thread graph + game state.

PROTOTYPE SCOPE (1–2 hafta MVP)

Minimum viable prototype should include:

ThreadSight overlay + drawing lines.

Node structs + ability to create/cut threads.

Simple causality engine: changing one thread updates target state (e.g., bridge stable/unstable).

One test level with bridge/enemy/plant to show multiple solutions.

Basic camera & controls.
This prototype is what you present at the tournament.

PLAY-LOOP METRICS (KPI)

Average puzzle solutions per level (target ≥ 3 different valid solutions).

Average session length (target 15–25 min for demo).

Emergent solution rate (how often players discover unpredicted solutions) — this is a key metric for novelty.

QA & PLAYTESTING

Early playtesters must be given only objectives, not hints. Measure time-to-solution and number of unique solutions.

Add telemetry to record thread operations to analyze emergent behavior.

NEXT STEPS (MENING TAKLIFIM — avtomatik, so‘rash kerak emas)

Immediate: Build MVP prototype (ThreadSight + cut/link + simple causality) in chosen stack (Vulkan + C++ core).

Week 1–2: Add 2–3 test puzzles, implement Node types (Physical & Emotion), add basic AI reaction.

Week 2–4: Polish visuals for thread overlay, add sound cues, expand level set.

Before tournament: Prepare 3-minute gameplay demo showcasing multiple solutions and emergent behavior.

Bu mening birinchi va katta oyinim bo'ladi!