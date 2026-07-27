/*
 * praatmex_gui_stubs.cpp
 *
 * Stub implementations for symbols that satisfy the linker for Praat's static
 * library objects that we exclude from the build (GUI, Formula, Picture, etc.).
 *
 * Uses forward declarations and a matching autoSomeThing<T> template whose
 * layout (single T* ptr) mirrors Praat's Thing.h.  Both definitions produce
 * weak / COMDAT symbols; the linker picks one.  Since every stub returns a
 * default-constructed (null) autoSomeThing, the destructor difference
 * (stubs: delete vs Praat: _Thing_forget) never fires — safe on MSVC,
 * GCC, and Clang.
 */

#include <cstdint>
#include <cstddef>
#include <wchar.h>

// ======================================================================
// Minimal Praat type forward declarations
// ======================================================================

typedef int64_t integer;
typedef char32_t *mutablestring32;
typedef const char32_t *conststring32;

struct structThing {
    void *classInfo;
    virtual ~structThing() noexcept {}
    virtual void v9_destroy() noexcept {}
};
typedef structThing *Thing;

struct structDaata : structThing {};
typedef structDaata *Daata;

struct structSound : structDaata {};
typedef structSound *Sound;

struct structTextGrid : structDaata {};
typedef structTextGrid *TextGrid;

struct structMelderFile {};
typedef structMelderFile *MelderFile;

struct structMelderReadText {};
typedef structMelderReadText *MelderReadText;

struct structGraphics {};
typedef structGraphics *Graphics;

struct structPicture : structThing {};
typedef structPicture *Picture;

struct structInterpreter {};
typedef structInterpreter *Interpreter;

struct structEditor : structThing {};
typedef structEditor *Editor;

struct structTextEditor : structEditor {};
typedef structTextEditor *TextEditor;

struct structScriptEditor : structEditor {};
typedef structScriptEditor *ScriptEditor;

struct structNotebook : structThing {};
typedef structNotebook *Notebook;

struct structNotebookEditor : structEditor {};
typedef structNotebookEditor *NotebookEditor;

struct structButtonEditor : structEditor {};
typedef structButtonEditor *ButtonEditor;

struct structDataEditor : structEditor {};
typedef structDataEditor *DataEditor;

struct structManPages {};
typedef structManPages *ManPages;

struct structScript {};
typedef structScript *Script;

struct structGuiDrawingArea {};
typedef structGuiDrawingArea *GuiDrawingArea;

struct structGuiWindow {};
typedef structGuiWindow *GuiWindow;

// ======================================================================
// autoSomeThing<T> — layout must match Praat's Thing.h (single T* ptr)
// so weak/COMDAT symbols resolve correctly across compilers.
// ======================================================================

template<class T>
class autoSomeThing {
    T *ptr;
public:
    autoSomeThing() : ptr(nullptr) {}
    autoSomeThing(T *p) : ptr(p) {}
    autoSomeThing(const autoSomeThing &o) = delete;
    autoSomeThing(autoSomeThing &&o) noexcept : ptr(o.ptr) { o.ptr = nullptr; }
    template<class Y>
    autoSomeThing(autoSomeThing<Y> &&o) noexcept : ptr(static_cast<T*>(o.get())) { o._zero(); }
    ~autoSomeThing() noexcept {
        if (ptr) { delete ptr; ptr = nullptr; }
    }
    T *get() const noexcept { return ptr; }
    void _zero() noexcept { ptr = nullptr; }
    void adoptFromAmbiguousOwner(T *p) noexcept { ptr = p; }
    T *releaseToAmbiguousOwner() noexcept { T *p = ptr; ptr = nullptr; return p; }
};

// ======================================================================
// Global variables
// ======================================================================

// Printer struct — must match the exact name for mangling
struct Printer {
    int spots;
    int paperSize;
    int orientation;
    bool postScript;
    bool allowDirectPostScript;
    int fontChoiceStrategy;
    long resolution;
    long paperWidth;
    long paperHeight;
    double magnification;
};

Printer thePrinter = {};

// ClassInfo — forward declare for classNotebook
struct structClassInfo {
    const char32_t *className;
    structClassInfo *semanticParent;
    integer size;
    Thing (*_new)();
    integer version;
    integer sequentialUniqueIdOfReadableClass;
    Thing dummyObject;
};
typedef structClassInfo *ClassInfo;

// classNotebook
ClassInfo classNotebook = nullptr;

// theClassInfo_DataGui
structClassInfo theClassInfo_DataGui = {};

// ======================================================================
// CollectionOf<structScriptEditor> — must match exact template name
// ======================================================================

template<typename T>
struct CollectionOf : structDaata {
    // Minimal layout: just needs to be a valid global variable
    int _dummy;
};

CollectionOf<structScriptEditor> theReferencesToAllOpenScriptEditors;

// ======================================================================
// Editor/TextEditor stubs — must use autoSomeThing<structXxx> for mangling
// ======================================================================

void TextEditor_showOpen(TextEditor) {}

autoSomeThing<structScriptEditor> ScriptEditor_createFromText(
    Editor, conststring32)
{
    return autoSomeThing<structScriptEditor>();
}

autoSomeThing<structScriptEditor> ScriptEditor_createFromScript_canBeNull(
    Editor, autoSomeThing<structScript>)
{
    return autoSomeThing<structScriptEditor>();
}

autoSomeThing<structNotebook> Notebook_createFromFile(MelderFile) {
    return autoSomeThing<structNotebook>();
}

autoSomeThing<structNotebookEditor> NotebookEditor_createFromText(
    conststring32)
{
    return autoSomeThing<structNotebookEditor>();
}

autoSomeThing<structNotebookEditor> NotebookEditor_createFromNotebook_canBeNull(
    Notebook)
{
    return autoSomeThing<structNotebookEditor>();
}

autoSomeThing<structButtonEditor> ButtonEditor_create() {
    return autoSomeThing<structButtonEditor>();
}

autoSomeThing<structDataEditor> DataEditor_create(conststring32, Daata) {
    return autoSomeThing<structDataEditor>();
}

// ---- Demo stubs ----
void Demo_saveToPdfFile(MelderFile) {}
bool Demo_hasGraphics(Graphics) { return false; }
void Demo_open() {}
void Demo_close() {}
void Demo_interpreterGoesAway(Interpreter) {}

// ---- Formula stubs ----
struct Formula_Result {
    int expressionType = 0;
    double numericResult = 0.0;
};

// ---- Picture stubs ----
autoSomeThing<structPicture> Picture_create(GuiDrawingArea, bool) {
    return autoSomeThing<structPicture>();
}

Graphics Picture_peekGraphics(Picture) { return nullptr; }

void Picture_setSelectionChangedCallback(Picture,
    void (*)(Picture, void *, double, double, double, double), void *) {}
void Picture_setMouseSelectsInnerViewport(Picture, int) {}
void Picture_erase(Picture) {}
void Picture_writeToPraatPictureFile(Picture, MelderFile) {}
void Picture_readFromPraatPictureFile(Picture, MelderFile) {}
void Picture_writeToEpsFile(Picture, MelderFile, bool, bool) {}
void Picture_writeToPdfFile(Picture, MelderFile) {}
void Picture_writeToPngFile_300(Picture, MelderFile) {}
void Picture_writeToPngFile_600(Picture, MelderFile) {}
void Picture_print(Picture) {}
void Picture_copyToClipboard(Picture) {}
void Picture_writeToWindowsMetafile(Picture, MelderFile) {}
void Picture_setSelection(Picture, double, double, double, double) {}

// ---- ManPages stubs ----
autoSomeThing<structManPages> ManPages_createFromText(MelderReadText, MelderFile) {
    return autoSomeThing<structManPages>();
}

// ---- zlib stubs (C linkage) ----
extern "C" {
typedef void *gzFile;
gzFile gzdopen(int, const char *) { return nullptr; }
int gzread(gzFile, void *, unsigned) { return -1; }
int gzclose(gzFile) { return -1; }
}

// ---- blake3 stubs (C linkage) ----
extern "C" {
struct blake3_hasher { uint8_t dummy[192]; };
void blake3_hasher_init(blake3_hasher *) {}
void blake3_hasher_update(blake3_hasher *, const void *, size_t) {}
void blake3_hasher_finalize(const blake3_hasher *, uint8_t *, size_t) {}
}

// ---- praat_statistics_prefs stub ----
void praat_statistics_prefs () {}

// ---- Photo_readFromImageFile stub ----
struct structPhoto : structDaata {};
typedef structPhoto *Photo;

autoSomeThing<structPhoto> Photo_readFromImageFile(MelderFile) {
    return autoSomeThing<structPhoto>();
}

// ---- Sound_and_TextGrid_extensions stubs (SpeechRecognizer/Silero excluded) ----
struct structIntervalTier {};
typedef structIntervalTier *IntervalTier;

autoSomeThing<structSound> Sound_IntervalTier_cutPartsMatchingLabel(
    structSound *, IntervalTier, conststring32)
{
    return autoSomeThing<structSound>();
}

autoSomeThing<structTextGrid> Sound_to_TextGrid_detectSilences(
    structSound *, double, double, double, double, double, conststring32, conststring32)
{
    return autoSomeThing<structTextGrid>();
}
