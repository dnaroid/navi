#ifndef UI_H
#define UI_H

#include <PolChar.h>
#include <utility>
#include <vector>

#define ANIM_MS 100

class Button {
public:
  Button(String text, const int x, const int y, const int w, const int h, const int id) :
    text(std::move(text)), x(x), y(y), w(w), h(h), id(id), updateAfterMs(0), _hidden(false),
    _type(' '), _disabled(false), _pressed(false), onPressCallback(nullptr) {
  }

  bool isCollide(int touchX, int touchY) const {
    return !_hidden && (touchX >= x && touchX <= x + w && touchY >= y && touchY <= y + h);
  }

  bool enabled() const {
    return !_disabled;
  }

  Button& enabled(const bool val) {
    _disabled = !val;
    return *this;
  }

  Button& visible(const bool val) {
    _hidden = !val;
    return *this;
  }

  bool visible() const {
    return !_hidden;
  }

  Button& caption(const String& newText) {
    text = newText;
    return *this;
  }

  String caption() {
    return text;
  }

  Button& onPress(const std::function<void(Button&)>& callback) {
    onPressCallback = callback;
    return *this;
  }

  Button& type(const char type) {
    _type = type;
    return *this;
  }

  char type() const {
    return _type;
  }

  String text;
  int x, y, w, h, id;
  unsigned long updateAfterMs;
  bool _hidden;
  char _type;
  bool _disabled;
  bool _pressed;
  std::function<void(Button&)> onPressCallback;
};

class Input {
public:
  Input(int x, int y, int w, int h, int id) : text(""), x(x), y(y), w(w), h(h), _hidden(false), _id(id) {
  }

  Input& addChar(char c) {
    text += c;
    return *this;
  }

  Input& removeChar() {
    if (text.length() > 0) {
      text.remove(text.length() - 1);
    }
    return *this;
  }

  Input& clear() {
    text = "";
    return *this;
  }

  Input& visible(bool val) {
    _hidden = !val;
    return *this;
  }

  bool visible() const {
    return !_hidden;
  }

  String text;
  int x, y, w, h;
  bool _hidden;
  int _id;
};

class UI {
public:
  UI(): updateAfterMs(0), _tft(nullptr), now(0), _nextId(10000) {
  }

  bool init(TFT_eSPI& tft) {
    _tft = &tft;
    return true;
  }

  Button& addButton(const Button& button) {
    buttons.push_back(button);
    return buttons.back();
  }

  Input& addInput(const Input& input) {
    inputs.push_back(input);
    return inputs.back();
  }

  Input& addInput(int x, int y, int w, int h, int id) {
    inputs.emplace_back(x, y, w, h, id);
    return inputs.back();
  }

  Button& addButton(const String& text, int x, int y, int w, int h, int id) {
    buttons.emplace_back(text, x, y, w, h, id);
    return buttons.back();
  }

  Button& addButton(const String& text, int x, int y, int w, int h) {
    buttons.emplace_back(text, x, y, w, h, _nextId++);
    return buttons.back();
  }

  Button& addButton(const char text, const int x, const int y) {
    return addButton(String(text), x, y, BUTTON_W, BUTTON_H, _nextId++);
  }

  Button& addButton(const char text, const int x, const int y, const int id) {
    return addButton(String(text), x, y, BUTTON_W, BUTTON_H, id);
  }

  void toggleBtnByType(const char type, const bool val) {
    for (auto& btn : buttons) if (btn._type == type) btn._hidden = !val;
  }

  void update() {
    now = millis();
    updateAfterMs = 0;
    for (auto& btn : buttons) drawButton(btn);
    for (auto& inp : inputs) drawInput(inp);
  }

  void drawButton(Button& btn) const {
    if (btn._hidden) return;
    if (btn._pressed && btn.updateAfterMs && now > btn.updateAfterMs) btn._pressed = false;
    if (btn._pressed) {
      _tft->fillRect(btn.x, btn.y, btn.w, btn.h, TFT_BLUE);
      _tft->setTextColor(TFT_WHITE);
    } else {
      _tft->fillRect(btn.x, btn.y, btn.w, btn.h, btn._disabled ? TFT_SILVER : TFT_WHITE);
      _tft->setTextColor(btn._disabled ? TFT_DARKGREY : TFT_BLACK);
    }
    if (btn.type() == 'a') {
      drawMultiLineText(btn.text, btn.x, btn.y, btn.w, btn.h, 1);
    } else {
      _tft->drawCentreString(btn.text, btn.x + btn.w / 2, btn.y + btn.h / 2 - 12, 4);
    }
  }

  void drawInput(const Input& inp) const {
    if (inp._hidden) return;
    _tft->fillRect(inp.x, inp.y, inp.w, inp.h, TFT_WHITE);
    _tft->setTextColor(TFT_BLACK);
    _tft->drawString(inp.text, inp.x, inp.y + 7, 1);
  }

  void drawMultiLineText(const String& text, const int x, const int y, const int w, const int h, const int font) const {
    const int charHeight = _tft->fontHeight(font) - 5;
    const int maxLines = h / charHeight;
    std::vector<String> lines;
    int start = 0;

    while (start < text.length()) {
      String line = text.substring(start, start + maxCharsPerLine);
      if (line.length() + start < text.length()) {
        if (line.length() == maxCharsPerLine && text.charAt(start + maxCharsPerLine) != ' ') {
          int lastSpace = line.lastIndexOf(' ');
          if (lastSpace != -1) {
            line = text.substring(start, start + lastSpace);
            start += lastSpace + 1;
          } else {
            start += maxCharsPerLine;
          }
        } else {
          start += maxCharsPerLine;
        }
      } else {
        start += maxCharsPerLine;
      }
      lines.push_back(line);
    }
    if (lines.size() > maxLines) {
      String lastLine = lines[maxLines - 1];
      lastLine = lastLine.substring(0, maxCharsPerLine - 3) + "...";
      lines[maxLines - 1] = lastLine;
      lines.resize(maxLines);
    }
    _tft->setTextDatum(MC_DATUM);
    for (int i = 0; i < lines.size(); i++) {
      _tft->drawString(utf8(lines[i].c_str()), x + w / 2, y + (i * charHeight) + 8, font);
    }
  }

  bool processPress(const int touchX, const int touchY) {
    now = millis();
    for (auto& b : buttons) {
      if (!b._hidden && b.isCollide(touchX, touchY)) {
        if (!b._disabled) {
          b._pressed = true;
          b.updateAfterMs = millis() + ANIM_MS;
          if (b.onPressCallback) b.onPressCallback(b);
          updateAfterMs = now + ANIM_MS * 2;
          drawButton(b);
        }
        return true;
      }
    }
    return false;
  }

  Button& findButtonByText(const String& text) {
    for (auto& btn : buttons) {
      if (btn.text == text) return btn;
    }
    Serial.print("Error: findButtonByText ");
    Serial.println(text);
    while (true);
  }

  Button& findButtonById(const int id) {
    for (auto& btn : buttons) {
      if (btn.id == id) return btn;
    }
    Serial.print("Error: findButtonById ");
    Serial.println(id);
    while (true);
  }

  Input& findInputById(const char id) {
    for (auto& inp : inputs) {
      if (inp._id == id) return inp;
    }
    Serial.print("Error: findInputById ");
    Serial.println(id);
    while (true);
  }

  void updateAfter(const int ms) {
    updateAfterMs = millis() + ms;
  }

  unsigned long updateAfterMs;
  static constexpr int maxCharsPerLine = 29;

private:
  TFT_eSPI* _tft;
  std::vector<Button> buttons;
  std::vector<Input> inputs;
  unsigned long now;
  int _nextId;
};

#endif //UI_H
