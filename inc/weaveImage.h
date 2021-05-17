#ifndef __WEAVEIMAGE_H_
#define __WEAVEIMAGE_H_
#include "threadpool.h"
#include "utils.h"
#include <SDL.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_thread.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <emmintrin.h>

using std::vector;
using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;
typedef union {
  int32_t i;
  struct {
    int16_t lo; // endian-specific!
    int16_t hi;
  };
} fixed_point;

struct pinSequence {
  uint_fast32_t first;
  uint_fast32_t second;
};
struct pinPair {
  uint_fast32_t first;
  SDL_Point fPos;
  uint_fast32_t second;
  SDL_Point sPos;
  vector<Uint8 *> pixels;
};
class WeaveImage {
public:
  WeaveImage(SDL_Renderer *rend, int winWidth, std::string file,
             uint_fast32_t numPins, uint_fast32_t numLines,
             uint_fast32_t pinOffset);
  void render(void);
  void drawPins();
  Uint8 getAlpha() { return _alpha; };
  void setAlpha(Uint8 alpha) { _alpha = alpha; };
  uint_fast32_t getNumPins() { return _numPins; };
  void setNumPins(uint_fast32_t val) { _numPins = val; };
  uint_fast32_t getMaxThreadsPins() { return _maxThreadCount; };
  void setMaxThreadsPins(uint_fast32_t val) { _maxThreadCount = val; };
  uint_fast32_t getLigten() { return _ligthenValue; };
  void set_Ligten(Uint8 val) {
    _ligthenValue128i = _mm_set1_epi8(val);
    _ligthenValue = val;
  };
  SDL_Renderer *getRen(void) { return _renderer; };
  uint_fast32_t getnumLinesDraw() { return _linesToDraw; };
  uint_fast32_t getPinOfsset() { return _pinOffset; };
  void setPinOfsset(uint_fast32_t val) { _pinOffset = val; };
  uint_fast32_t getnumLines() { return _numLines; };
  void setnumLines(uint_fast32_t val) { _numLines = val; };
  void setLinesToDraw(uint_fast32_t in);
  void ligthChangeCallBack();
  void recalAllCallBack();
  void openNewFileCallBack(std::string file);
  void saveFileCallBack(std::string file);
  void saveBinFileCallBack(std::string file);
  void printPinSequence(void);

private:
  // Thread pool to calculate score
  // boost::asio::thread_pool *workers;
  thread_pool *pool;
  uint_fast32_t _numThreads;
  std::string _file;
  SDL_Renderer *_renderer = NULL;
  uint_fast32_t _pinOffset; // Numbers of pins near current pin not to weav
  uint_fast32_t _linesToDraw;
  Uint8 _alpha;
  Uint8 _ligthenValue;
  __m128i _ligthenValue128i;
  uint_fast32_t _pinStride;
  uint_fast32_t _maxThreadCount;
  int _winWidth, _winHeigth;
  SDL_Texture *_texture = NULL;
  SDL_Surface *_surface = NULL;
  vector<vector<Uint8>> _greyOri;
  vector<vector<Uint8>> _greyWorkImg;
  SDL_Rect _textRect;
  float _windowScale;
  SDL_PixelFormat _pixelFormat;
  uint_fast32_t _numPins;
  uint_fast32_t _numLines;               // Maximum number of lines
  vector<pinSequence> _pinSequence;      // Finale sequence to sow
  vector<uint_fast32_t> _pinThreadCount; // Finale sequence to sow
  vector<SDL_Point> _pinPos;
  vector<pinPair> _pinPair;

  void _generateLinePixelsPointers();
  void _getLinePixelPointer(uint_fast32_t index);
  void _generateGreyVector();
  void _generatePinPosition(); // Cal the point location
  void _generatePinPairs();    // Cal the pin pairs to verify

  void _drawLines();
  uint_fast32_t _getBestScoreIndexThreaded(const uint_fast32_t pin);
  uint_fast32_t _getBestScoreIndex(uint_fast32_t pin);
  void _ligthenImagePointer( uint_fast32_t index);
  void _getPinSequence(void);
};

#endif // __WEAVEIMAGE_H_
