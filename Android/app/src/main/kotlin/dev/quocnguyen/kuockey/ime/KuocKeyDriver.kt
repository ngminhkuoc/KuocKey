package dev.quocnguyen.kuockey.ime

/** An edit the engine wants applied before the cursor (Engine/Driver.h VKEdit). */
class Edit(val deleteCount: Int, val text: String, val languageChanged: Boolean)

/** A word the fat-finger model or the completer offers (Engine/TouchModel.h TMCandidate). */
class Candidate(val letters: String, val text: String, val vietnamese: Boolean, val score: Double) {
    fun packed() = "$letters\t$text\t${if (vietnamese) 1 else 0}\t$score"
    companion object {
        fun unpack(s: String): Candidate {
            val p = s.split('\t')
            return Candidate(p[0], p.getOrElse(1) { "" }, p.getOrElse(2) { "0" } == "1", p.getOrElse(3) { "0" }.toDoubleOrNull() ?: 0.0)
        }
    }
}

/** JNI face of Engine/Driver + TouchModel (Android/app/src/main/cpp/kuockey_jni.cpp). */
object KuocKeyDriver {
    init { System.loadLibrary("kuockey") }
    @JvmStatic external fun create(): Long
    @JvmStatic external fun destroy(h: Long)
    @JvmStatic external fun language(h: Long): Int
    @JvmStatic external fun setLanguage(h: Long, l: Int)
    @JvmStatic external fun configure(h: Long, autoSwitch: Boolean, pause: Double, spelling: Boolean, restore: Boolean, modern: Boolean)
    @JvmStatic external fun setTelexStyle(style: Int)
    @JvmStatic external fun typeChar(h: Long, c: Char, now: Double): String
    @JvmStatic external fun backspace(h: Long, now: Double): String
    @JvmStatic external fun manualSwitch(h: Long, now: Double)
    @JvmStatic external fun resetContext(h: Long)
    @JvmStatic external fun pickLetter(x: Float, y: Float, prefix: String, vi: Int, letters: ByteArray, rects: FloatArray): Byte
    @JvmStatic external fun candidates(typed: String, xs: FloatArray?, ys: FloatArray?, vi: Int, letters: ByteArray, rects: FloatArray, max: Int): Array<String>
    @JvmStatic external fun shouldCorrect(typed: String, cands: Array<String>): Boolean
    @JvmStatic external fun completions(prefix: String, vi: Int, max: Int): Array<String>
    @JvmStatic external fun wordScore(vi: Int, letters: String): Int
}

/** The letter keys of the layout, for the touch model: one byte + one rect (x, y, w, h) per key. */
class KeyRects(val letters: ByteArray, val rects: FloatArray) {
    val isEmpty get() = letters.isEmpty()
}

/** Mirrors iOS/Shared/VKDriver: one engine per keyboard. */
class Driver {
    private val h = KuocKeyDriver.create()
    var autoSwitch = true
    var pauseAfterManual = 5.0
    var modernOrthography = true
    var telexStyle = 0   // D56: 0 Telex, 1 Simple Telex 1, 2 Simple Telex 2
    var spelling = true
    var restoreIfInvalid = true
    var language: Int
        get() = KuocKeyDriver.language(h)
        set(v) = KuocKeyDriver.setLanguage(h, v)

    fun applySettings() { KuocKeyDriver.configure(h, autoSwitch, pauseAfterManual, spelling, restoreIfInvalid, modernOrthography); KuocKeyDriver.setTelexStyle(telexStyle) }
    private fun edit(s: String) = Edit(s[0].code, s.substring(2), s[1].code == 1)
    /** One character typed: letters, digits, punctuation, " " or "\n". Anything the engine doesn't know passes through. */
    fun typeCharacter(s: String, now: Double): Edit {
        if (s.length != 1) { resetContext(); return Edit(0, s, false) }
        return edit(KuocKeyDriver.typeChar(h, s[0], now))
    }
    fun backspace(now: Double) = edit(KuocKeyDriver.backspace(h, now))
    fun manualSwitch(now: Double) = KuocKeyDriver.manualSwitch(h, now)
    fun resetContext() = KuocKeyDriver.resetContext(h)
    fun close() = KuocKeyDriver.destroy(h)

    companion object {
        fun pickLetter(x: Float, y: Float, prefix: String, vietnamese: Boolean, keys: KeyRects): Char? {
            val b = KuocKeyDriver.pickLetter(x, y, prefix, if (vietnamese) 1 else 0, keys.letters, keys.rects)
            return if (b.toInt() == 0) null else b.toInt().toChar()
        }
        fun candidates(typed: String, xs: FloatArray?, ys: FloatArray?, vietnamese: Boolean, keys: KeyRects, max: Int = 6) =
            KuocKeyDriver.candidates(typed, xs, ys, if (vietnamese) 1 else 0, keys.letters, keys.rects, max).map(Candidate::unpack)
        fun shouldCorrect(typed: String, c: List<Candidate>) = KuocKeyDriver.shouldCorrect(typed, c.map { it.packed() }.toTypedArray())
        fun completions(prefix: String, vietnamese: Boolean, max: Int = 6) = KuocKeyDriver.completions(prefix, if (vietnamese) 1 else 0, max).map(Candidate::unpack)
        fun wordScore(vietnamese: Boolean, letters: String) = KuocKeyDriver.wordScore(if (vietnamese) 1 else 0, letters)
    }
}
