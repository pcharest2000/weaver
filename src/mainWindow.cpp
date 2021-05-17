#include "mainWindow.h"
#define FONT_SIZE 12
#define SLIDER_WIDTH 400

#define MAX_LINES 8000
#define MIN_LINES 1000
#define MAX_PINS 500
#define MIN_PINS 100
#define MAX_THREADS 100
#define MIN_THREADS 5
#define MAX_OFFSET 50
MainWindow::MainWindow(SDL_Window *pwindow, WeaveImage *weaver, int width,
                       int heigth)
    : Screen(pwindow, Vector2i(width, heigth), "Portrait Weaver") {
  _width = width;
  _heigth = heigth;
  _weaver = weaver;
  {
    auto &nwindow = window("Weaver", Vector2i{0, 0}).withLayout<GroupLayout>();
    Theme *theme = nwindow.theme();
    theme->mWindowDropShadowSize = 0;
    theme->mWindowHeaderHeight = 20;
    theme->mStandardFontSize = FONT_SIZE;
    theme->mTextBoxFontSize = FONT_SIZE;
    theme->mButtonFontSize = FONT_SIZE;

    auto &popPar = nwindow.popupbutton("Weave Parameters");
    auto &popDis = nwindow.popupbutton("Display Parameters");

    popDis.popup()
        .withLayout<GroupLayout>()
        .label("Num Lines To Draw", "sans-bold")
        ._and()
        .widget()
        .withLayout<BoxLayout>(Orientation::Horizontal, Alignment::Fill, 0, 20)
        .slider((float)_weaver->getnumLinesDraw() / _weaver->getnumLines(),
                [this](Slider *obj, float value) {
                  this->_weaver->setLinesToDraw(value *
                                                this->_weaver->getnumLines());
                  if (auto *textBox = obj->gfind<TextBox>("numStringDraw"))
                    textBox->setValue(
                        std::to_string((this->_weaver->getnumLinesDraw())));
                },
                [this](float value) {})
        .withFixedWidth(SLIDER_WIDTH)
        ._and()
        .textbox(std::to_string(this->_weaver->getnumLinesDraw()))
        .withAlignment(TextBox::Alignment::Right)
        .withId("numStringDraw")
        .withFixedSize(Vector2i(50, 25));

    popDis.popup()
        .withLayout<GroupLayout>()
        .label("Line alpha value", "sans-bold")
        ._and()
        .widget()
        .withLayout<BoxLayout>(Orientation::Horizontal, Alignment::Fill, 0, 20)
        .slider((float)this->_weaver->getAlpha() / 255.0,
                [this](Slider *obj, float value) {
                  this->_weaver->setAlpha(value * 255);
                  if (auto *textBox = obj->gfind<TextBox>("Alpha"))
                    textBox->setValue(std::to_string((int)(value * 255.0)));
                },
                [this](float value) { this->_weaver->setAlpha(value * 255); })
        .withFixedWidth(SLIDER_WIDTH)
        ._and()
        .textbox(std::to_string((int)this->_weaver->getAlpha()))
        .withAlignment(TextBox::Alignment::Right)
        .withId("Alpha")
        .withFixedSize(Vector2i(50, 25));

    popPar.popup()
        .withLayout<GroupLayout>()
        .label("Max numbers of Lines", "sans-bold")
        ._and()
        .widget()
        .withLayout<BoxLayout>(Orientation::Horizontal, Alignment::Fill, 0, 20)
        .slider((float)_weaver->getnumLines() / (MAX_LINES - MIN_LINES),
                [this](Slider *obj, float value) {
                  if (auto *textBox = obj->gfind<TextBox>("numString"))
                    textBox->setValue(std::to_string(
                        (int)(value * (MAX_LINES - MIN_LINES) + MIN_LINES)));
                },
                [this](float value) {
                  this->_weaver->setnumLines(value * (MAX_LINES - MIN_LINES) +
                                             MIN_LINES);
                  this->_weaver->recalAllCallBack();
                })
        .withFixedWidth(SLIDER_WIDTH)
        ._and()
        .textbox(std::to_string((int)(_weaver->getnumLines())))
        .withAlignment(TextBox::Alignment::Right)
        .withId("numString")
        .withFixedSize(Vector2i(50, 25));

    popPar.popup()
        .withLayout<GroupLayout>()
        .label("Number of pins", "sans-bold")
        ._and()
        .widget()
        .withLayout<BoxLayout>(Orientation::Horizontal, Alignment::Fill, 0, 20)
        .slider(((float)_weaver->getNumPins()) / (MAX_PINS - MIN_PINS),
                [this](Slider *obj, float value) {
                  this->_weaver->setNumPins((MAX_PINS - MIN_PINS) * value +
                                            MIN_PINS);
                  if (auto *textBox = obj->gfind<TextBox>("numPin"))
                    textBox->setValue(
                        std::to_string((int)this->_weaver->getNumPins()));
                },
                [this](float value) {
                  this->_weaver->setNumPins((MAX_PINS - MIN_PINS) * value +
                                            MIN_PINS);
                  this->_weaver->recalAllCallBack();
                })
        .withFixedWidth(SLIDER_WIDTH)
        ._and()
        .textbox(std::to_string(_weaver->getNumPins()))
        .withAlignment(TextBox::Alignment::Right)
        .withId("numPin")
        .withFixedSize(Vector2i(50, 25));

    popPar.popup()
        .withLayout<GroupLayout>()
        .label("Ligthen value", "sans-bold")
        ._and()
        .widget()
        .withLayout<BoxLayout>(Orientation::Horizontal, Alignment::Fill, 0, 20)
        .slider(
            _weaver->getLigten() / 255.0,
            [this](Slider *obj, float value) {
              this->_weaver->set_Ligten(value * 254 + 1);
              if (auto *textBox = obj->gfind<TextBox>("lignValue"))
                textBox->setValue(std::to_string(this->_weaver->getLigten()));
            },
            [this](float value) { this->_weaver->ligthChangeCallBack(); })
        .withFixedWidth(SLIDER_WIDTH)
        ._and()
        .textbox(std::to_string(_weaver->getLigten()))
        .withAlignment(TextBox::Alignment::Right)
        .withId("lignValue")
        .withFixedSize(Vector2i(50, 25));

    popPar.popup()
        .withLayout<GroupLayout>()
        .label("Max threads per pins", "sans-bold")
        ._and()
        .widget()
        .withLayout<BoxLayout>(Orientation::Horizontal, Alignment::Fill, 0, 20)
        .slider(
            (float)_weaver->getMaxThreadsPins() / (MAX_THREADS - MIN_THREADS),
            [this](Slider *obj, float value) {
              this->_weaver->setMaxThreadsPins(
                  value * (MAX_THREADS - MIN_THREADS) + MIN_THREADS);
              if (auto *textBox = obj->gfind<TextBox>("maxthread"))
                textBox->setValue(
                    std::to_string(this->_weaver->getMaxThreadsPins()));
            },
            [this](float value) { this->_weaver->ligthChangeCallBack(); })
        .withFixedWidth(SLIDER_WIDTH)
        ._and()
        .textbox(std::to_string(_weaver->getMaxThreadsPins()))
        .withAlignment(TextBox::Alignment::Right)
        .withId("maxthread")
        .withFixedSize(Vector2i(50, 25));
    popPar.popup()
        .withLayout<GroupLayout>()
        .label("Pin offset", "sans-bold")
        ._and()
        .widget()
        .withLayout<BoxLayout>(Orientation::Horizontal, Alignment::Fill, 0, 20)
        .slider(
            (float) _weaver->getPinOfsset() / (MAX_OFFSET),
            [this](Slider *obj, float value) {
              this->_weaver->setPinOfsset(value * MAX_OFFSET);
              if (auto *textBox = obj->gfind<TextBox>("maxoffset"))
                textBox->setValue(
                    std::to_string(this->_weaver->getPinOfsset()));
            },
            [this](float value) { this->_weaver->recalAllCallBack(); })
        .withFixedWidth(SLIDER_WIDTH)
        ._and()
        .textbox(std::to_string(_weaver->getPinOfsset()))
        .withAlignment(TextBox::Alignment::Right)
        .withId("maxoffset")
        .withFixedSize(Vector2i(50, 25));

    // nwindow.label("File dialog", "sans-bold")
    //     ._and()
       nwindow.widget()
        .boxlayout(Orientation::Horizontal, Alignment::Minimum, 0, 6)
        .button("Open",
                [this] {
                  std::string file =
                      file_dialog({{"png", "Portable Network Graphics"},
                                   {"jpeg", "Joint Photographic Experts Group"},
                                   {"jpg", "Joint Photographic Experts Group"}},
                                  false);
                  if (file.size() == 1) {
                    return;
                  }
                  this->_weaver->openNewFileCallBack(file);
                })
        .withTooltip("Open new image")
        ._and()
        .button("Save TXT", [this] {
            std::string file=file_dialog({{"txt", "Text file"}}, true);
            this->_weaver->saveFileCallBack(file);
        })
        .withTooltip("Save pin sequence in text file")
        ._and()
        .button("Save BIN", [this] {
            std::string file=file_dialog({{"bin", "Bin"}}, true);
            this->_weaver->saveBinFileCallBack(file);
        })
        .withTooltip("Save pin sequence in binary unsigned 16 bit");
  }

  /* Create an empty panel with a horizontal layout */
  performLayout(mSDL_Renderer);
}
