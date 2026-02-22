# Plugin Design & Development Template

This document serves as a starting point and set of guiding principles for designing and developing audio plugins. Its purpose is to foster creativity, ensure quality, and maintain a cohesive design language across all our products.

---

## 1. Core Philosophy

*   **Simplicity & Intuitiveness:** Plugins should be easy to understand and use. The UI should be clean, and the workflow logical. What you see is what you get.
*   **Musicality:** Every feature should serve a musical purpose. Parameters should have useful, musical ranges. The plugin should sound good and inspire creativity.
*   **Performance:** Plugins must be CPU-efficient and stable across all major DAWs.
*   **Cohesion:** All plugins in our suite should share a common design language, making them feel familiar to users of our other products.

---

## 2. The "Big Idea" - Conceptualization

* **Plugin Name:** `a LOUDER Saturator`
* **One-Liner:** Add mud-free saturation to your drum bus, mix bus, or master chain to get that essential bite without ruining your mix.
* **Target User:** Producers of all genres looking for a versatile "glue" to tie their sonic elements together.
* **Core Problem:** Finding a simple, effective way to bring cohesion and bite to a mix without introducing muddy frequencies.
* **Key Differentiator:** A built-in room reverb with a pre/post routing toggle. This allows users to emulate the sound of instruments recorded on a room mic and *then* driven through a hardware saturator, bringing out an aggressive, glued room tone.

---

## 3. Research & Development (R&D)

*Good decisions are informed by good research. Before committing to a feature set or DSP approach, investigate the landscape.*

*   **Hardware Inspiration:**
    *   Are there any classic hardware units this plugin is inspired by? (e.g., Urei 1176, Teletronix LA-2A, Pultec EQP-1A).
    *   What are the key characteristics of that hardware? (e.g., specific component behaviors like tube saturation, transformer hysteresis, optical cell attack/release curves).
    *   How can we model or emulate these desirable characteristics?

*   **Software & Academic Precedent:**
    *   What are the top competing plugins in this category?
    *   What do they do well? What are their weaknesses?
    *   Are there any relevant academic papers or DSP articles (e.g., from DAFx, AES, or blogs) that discuss algorithms for this type of effect?

*   **Key Learnings & Takeaways:**
    *   Summarize the most important findings from the research.
    *   *e.g., "The LA-2A's program-dependent release is its most beloved feature. We must get this right."*
    *   *e.g., "Most competing plugins lack a wet/dry mix control. Adding one would be a significant advantage."*

---

## 4. Feature Set

| Control/Feature | Description | Parameter Type & Range | Default Value |
| :--- | :--- | :--- | :--- |
| **Drive** | Sets the amount of saturation applied to the signal. | Rotary Knob / Float / 0.0 to 10.0 | 0.0 |
| **Reverb** | Controls the amount (mix/decay) of the built-in room tone. | Rotary Knob / Float / 0% to 100% | 0% |
| **Pre/Post** | Determines if the Reverb is applied *before* the Saturator (glued room tone) or *after* (clean room tone). | Button / Toggle / Pre or Post | Pre |
| **Tone** | A tilt-style EQ (Low/High shelf balance) to color the saturation and keep the low-end mud-free. | Rotary Knob / Float / -100 to +100 | 0 (Flat) |
| **Width** | Controls the Mono/Stereo spread of the wet signal. | Rotary Knob / Float / 0% (Mono) to 200% (Extra Wide) | 100% (Stereo) |
| **Mix** | Dry/Wet blend between the completely unaffected input and the processed chain. | Rotary Knob / Float / 0% to 100% | 100% |
| **Output** | Final makeup gain to volume-match the processed signal with the dry signal. | Rotary Knob / Float / -24dB to +24dB | 0dB |

---

## 5. User Interface & Experience (UI/UX)

* **Design Inspiration:** Teenage Engineering, Elektron, UDO Audio. 
* **Overall Style:** Utilitarian, sleek, minimalist, and flat. Zero skeuomorphism (no drop shadows, no 3D glare, no faux-metal). Strict geometric lines and clean, technical typography.

* **Layout & Visual Hierarchy:**
    * **Hero Control (Large):** Drive (Center focus).
    * **Secondary Controls (Medium):** Reverb, Tone (Flanking the Drive knob).
    * **Tertiary Controls (Small):** Mix, Width, Output (Arranged neatly below).
    * **Toggle:** A sleek, flat, modern rectangular button for Pre/Post routing.
    * **Metering:** Minimalist vertical LED bar meters (Input on the far left edge, Output on the far right edge).

* **Color Palette:**
    * **Background:** Matte Dark Grey/Soft Black (e.g., `juce::Colour::fromString("#1A1A1A")` or `(26, 26, 26)`).
    * **Accent Color:** Bright Electric Blue (`#0087ff` or `juce::Colour::fromString("#0087ff")`). Used for knob values, meter peaks, and active toggles.
    * **Track/Empty Rings:** Dark Grey (`#333333`).
    * **Text/Labels:** Off-White/Light Grey (`#E0E0E0`).

* **Typography:**
    * **Font Style:** Clean, technical sans-serif or monospaced (to fit the Elektron vibe).
    * **Title:** Minimalist, perhaps placed in the top-left corner.
    * **Labels:** Small, crisp, all-caps.

* **Component Rules (Custom LookAndFeel):**
    * **Rotary Sliders:** Flat, solid-color arcs to represent the value, with a simple dot or straight line for the pointer. No gradients.

---

## 6. Iteration & Refinement

_Use this section when improving an existing plugin._

*   **Plugin to Iterate:** `[Name of the existing plugin]`
*   **Current Weaknesses:** What are the common complaints, limitations, or areas for improvement?
    *   *e.g., "The attack time is too slow for percussive material."*
    *   *e.g., "The UI feels cluttered."*
*   **Proposed Improvements:**
    1.  *e.g., Add a "Fast Attack" toggle button.*
    2.  *e.g., Redesign the UI to group related controls together.*
    3.  *e.g., Improve CPU usage by optimizing the DSP algorithm.*
*   **Justification:** Why are these changes necessary? How do they align with our core philosophy?

---

## 7. Cohesion Checklist

_Before finalizing a design, ensure it aligns with our other plugins._

- [ ] Does it use the standard color palette?
- [ ] Does it use the standard fonts and typography?
- [ ] Is knob/slider behavior consistent (e.g., drag direction, value mapping)?
- [ ] Are naming conventions for parameters consistent (e.g., "Gain", "Mix", "Drive")?
- [ ] Does the overall look and feel match our brand identity?
- [ ] Does it solve a clear problem without feature bloat?
