#include "weaveImage.h"

WeaveImage::WeaveImage(SDL_Renderer *rend, int winWidth, std::string file,
                       uint_fast32_t numPins, uint_fast32_t numLines,
                       uint_fast32_t pinOffset) {
  /*
   * This will add 2 threads to the thread pool. (You could just put it in a for
   * loop)
   */
  _numThreads = std::thread::hardware_concurrency();

  std::cout << "Available Threads: 2" << _numThreads << '\n';
  pool = new thread_pool(2);
  _linesToDraw = numLines;
  _renderer = rend;
  _alpha = 100;
  _ligthenValue = 20;
  _ligthenValue128i = _mm_set1_epi8(_ligthenValue);
  _maxThreadCount = 20;
  _pinThreadCount.resize(numPins, 0);
  _pinOffset = pinOffset;
  _numPins = numPins;
  _numLines = numLines;
  _pinStride = _numPins - 2 * _pinOffset;
  _winWidth = winWidth;
  _file = file;
  _surface = IMG_Load(_file.c_str());
  _texture = IMG_LoadTexture(_renderer, _file.c_str());
  SDL_QueryTexture(_texture, &_pixelFormat.format, nullptr, &_textRect.w,
                   &_textRect.h);
  _windowScale = (float)_winWidth / (float)_surface->w;

  auto t1 = high_resolution_clock::now();
  auto t2 = high_resolution_clock::now();
  duration<double, std::milli> ms_double = t2 - t1;

  std::cout << "Genrating Grey vector: " << std::endl;
  t1 = high_resolution_clock::now();
  _generateGreyVector();
  t2 = high_resolution_clock::now();
  ms_double = t2 - t1;
  std::cout << ms_double.count() << "ms" << std::endl;

  std::cout << "Genrating Line indexes: " << std::endl;
  t1 = high_resolution_clock::now();
  _generatePinPosition();
  _generatePinPairs();
  _generateLinePixelsPointers();
  t2 = high_resolution_clock::now();
  ms_double = t2 - t1;
  std::cout << ms_double.count() << "ms" << std::endl;

  std::cout << "Get pin Sequence" << std::endl;
  t1 = high_resolution_clock::now();
  _getPinSequence();
  t2 = high_resolution_clock::now();
  ms_double = t2 - t1;
  std::cout << ms_double.count() << "ms" << std::endl << std::flush;
}

void WeaveImage::ligthChangeCallBack() {
  _greyWorkImg = _greyOri;
  _pinSequence.clear();
  _pinThreadCount.clear();
  _pinThreadCount.resize(_numPins, 0);
  _getPinSequence();
}
void WeaveImage::recalAllCallBack() {
  _greyWorkImg = _greyOri;
  _pinSequence.clear();
  _pinThreadCount.clear();
  _pinThreadCount.resize(_numPins, 0);
  _pinPos.clear();
  _pinPair.clear();
  _linesToDraw = _numLines;
  _pinStride = _numPins - 2 * _pinOffset;
  _generatePinPosition();
  _generatePinPairs();
  _generateLinePixelsPointers();
  _getPinSequence();
}
void WeaveImage::setLinesToDraw(uint_fast32_t in) {
  if (in > _numLines) {
    _linesToDraw = _numLines;
    return;
  }
  _linesToDraw = in;
}
void WeaveImage::render(void) {
  SDL_RenderSetScale(_renderer, _windowScale, _windowScale);
  _drawLines();
  SDL_RenderSetScale(_renderer, 1, 1);
}
void WeaveImage::_generatePinPosition() {
  SDL_Point pos;
  float angleSteps = M_PI * 2 / _numPins;
  float middle = floor(_surface->w / 2.0 - 0.5);
  for (int i = 0; i < _numPins; i++) {
    pos.x = (cosf(i * angleSteps) * middle + middle);
    pos.y = (middle - sinf(i * angleSteps) * middle);
    _pinPos.push_back(pos);
  }
}
void WeaveImage::_generatePinPairs() {
  pinPair pair;
  for (uint_fast32_t i = 0; i < _numPins; i++) {
    pair.second = addSaturate(i, _pinOffset, _numPins - 1);
    pair.sPos.x = _pinPos[pair.second].x;
    pair.sPos.y = _pinPos[pair.second].y;
    for (uint_fast32_t j = 0;
         j < (const uint_fast32_t)(_numPins - 2 * _pinOffset); j++) {
      pair.first = i;
      pair.fPos.x = _pinPos[i].x;
      pair.fPos.y = _pinPos[i].y;
      _pinPair.push_back(pair);
      pair.second = addSaturate(pair.second, 1, _numPins - 1);
      pair.sPos.x = _pinPos[pair.second].x;
      pair.sPos.y = _pinPos[pair.second].y;
    }
  }
}

void WeaveImage::_generateLinePixelsPointers() {
  pool->push_task([&]() {
    for (uint_fast32_t i = 0; i < (const uint_fast32_t)_pinPair.size() / 2;
         i++) {
      _getLinePixelPointer(i);
    }
  });
  pool->push_task([&]() {
    for (uint_fast32_t i = _pinPair.size() / 2;
         i < (const uint_fast32_t)_pinPair.size(); i++) {
      _getLinePixelPointer(i);
    }
  });
    // for (uint_fast32_t i = _pinPair.size() / 2;
    //      i < (const uint_fast32_t)_pinPair.size(); i++) {
    //   _getLinePixelPointer(i);
    // }
  pool->wait_for_tasks();
}

void WeaveImage::drawPins() {
  for (vector<SDL_Point>::iterator it = _pinPos.begin(); it != _pinPos.end();
       ++it) {
    SDL_RenderDrawPoint(_renderer, it->x, it->y);
  }
}

void WeaveImage::_getLinePixelPointer(uint_fast32_t index) {
  int x0 = _pinPair[index].fPos.x;
  int y0 = _pinPair[index].fPos.y;
  int x1 = _pinPair[index].sPos.x;
  int y1 = _pinPair[index].sPos.y;
  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = (dx > dy ? dx : -dy) / 2, e2;
  for (;;) {
    _pinPair[index].pixels.push_back(&_greyWorkImg[x0][y0]);
    if (x0 == x1 && y0 == y1)
      break;
    e2 = err;
    if (e2 > -dx) {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dy) {
      err += dx;
      y0 += sy;
    }
  }
}

void WeaveImage::_drawLines() {
  SDL_SetRenderDrawColor(_renderer, 0, 0, 0, _alpha);
  for (int i = 0; i < _linesToDraw; i++) {
    SDL_RenderDrawLine(_renderer, _pinPos[_pinSequence[i].first].y,
                       _pinPos[_pinSequence[i].first].x,
                       _pinPos[_pinSequence[i].second].y,
                       _pinPos[_pinSequence[i].second].x);
  }
}
void WeaveImage::_generateGreyVector() {
  SDL_Surface *oriSurface = _surface;
  _surface = SDL_ConvertSurfaceFormat(_surface, SDL_PIXELFORMAT_ARGB8888, 0);
  SDL_LockSurface(_surface);
  Uint32 *pixels = (Uint32 *)_surface->pixels;
  const int w = _surface->w;
  const int h = _surface->h;
  //_greyOri.reserve(h);
  //_greySurface.resize(w);
  for (uint_fast32_t i = 0; i < h; i++) {
    vector<Uint8> v1(w);
    for (uint_fast32_t j = 0; j < w; j++) {
      Uint8 b = *pixels & 0xFF;
      v1[j] = b;
      pixels++;
    }
    _greyWorkImg.push_back(v1);
    _greyOri.push_back(v1);
  }
  //  _greyWorkImg = _greyOri; // Keep a copy of orginale...
  SDL_UnlockSurface(_surface);
  SDL_FreeSurface(oriSurface);
}
void WeaveImage::_getPinSequence(void) {
  // We start at random pin 0
  uint_fast32_t bestIndex = 0;
  uint_fast32_t prevBestPin = 0;
  pinSequence tmp;
  tmp.first = 0;
  tmp.second = 0;
  for (uint32_t i = 0; i < (const uint_fast32_t)(_numLines); i++) {
    tmp.first = tmp.second;
    bestIndex = _getBestScoreIndexThreaded(tmp.second);
    tmp.second = _pinPair[bestIndex].second;

    _pinSequence.push_back(tmp);
    _pinThreadCount[_pinPair[bestIndex].first]++;
    _ligthenImagePointer(bestIndex);
  }
}

uint_fast32_t WeaveImage::_getBestScoreIndex(uint_fast32_t pin) {
  const uint_fast32_t index = pin * _pinStride;
  uint_fast32_t sum = 0;
  uint_fast32_t bestIndex = index;
  float score;
  float bestScore = 1E10;
  for (uint_fast32_t i = index; i < (const uint_fast32_t)(index + _pinStride);
       i++) {
    sum = 0;
    for (uint_fast32_t j = 0; j < _pinPair[i].pixels.size(); j++) {
      sum += *_pinPair[i].pixels[j];
    }
    score = float(sum) / _pinPair[i].pixels.size();
    if (score < bestScore) {
      bestScore = score;
      bestIndex = i;
    }
  }
  return bestIndex;
}

uint_fast32_t WeaveImage::_getBestScoreIndexThreaded(const uint_fast32_t pin) {

  const uint_fast32_t index = pin * _pinStride;
  const uint_fast32_t index2 = pin * _pinStride + _pinStride / 2;
  uint_fast32_t bestIndex = index;
  float bestScore = 1E10;
  uint_fast32_t bestIndex2 = index;
  float bestScore2 = 1E10;

  pool->push_task([&]() {
    uint_fast32_t sum = 0;
    float score;
    for (uint_fast32_t i = index;
         i < (const uint_fast32_t)(index + _pinStride / 2); i++) {
      sum = 0;
      const uint_fast32_t size = _pinPair[i].pixels.size();
      for (uint_fast32_t j = 0; j < size; j++) {
        sum += *_pinPair[i].pixels[j];
      }
      score = float(sum) / size;
      if (score < bestScore && _pinThreadCount[pin] <= _maxThreadCount) {
        bestScore = score;
        bestIndex = i;
      }
    }
  });
  // pool->push_task([&]() {
  //   uint_fast32_t sum = 0;
  //   float score;
  //   for (uint_fast32_t i = index2;
  //        i < (const uint_fast32_t)(index2 + _pinStride / 2); i++) {
  //     const uint_fast32_t size = _pinPair[i].pixels.size();
  //     sum = 0;
  //     for (uint_fast32_t j = 0; j < size; j++) {
  //       sum += *_pinPair[i].pixels[j];
  //     }
  //     score = float(sum) / size;
  //     if (score < bestScore2 && _pinThreadCount[pin] <= _maxThreadCount) {
  //       bestScore2 = score;
  //       bestIndex2 = i;
  //     }
  //   }
  // });
    uint_fast32_t sum = 0;
    float score;
    for (uint_fast32_t i = index2;
         i < (const uint_fast32_t)(index2 + _pinStride / 2); i++) {
      const uint_fast32_t size = _pinPair[i].pixels.size();
      sum = 0;
      for (uint_fast32_t j = 0; j < size; j++) {
        sum += *_pinPair[i].pixels[j];
      }
      score = float(sum) / size;
      if (score < bestScore2 && _pinThreadCount[pin] <= _maxThreadCount) {
        bestScore2 = score;
        bestIndex2 = i;
      }
    }
  pool->wait_for_tasks();
  if (bestScore < bestScore2) {
    return bestIndex;
  }
  return bestIndex2;
}
void WeaveImage::_ligthenImagePointer(uint_fast32_t index) {
  uint32_t pixindex=0;
  uint32_t pixindex2=0;
  __m128i result;
  __m128i value;
  uint8_t *ptr = (uint8_t *)&result;
  uint8_t *ptrt = (uint8_t *)&result;
  uint8_t *ptv = (uint8_t *)&value;
  uint8_t *ptvt = (uint8_t *)&value;
  uint32_t indexStride = _pinPair[index].pixels.size() / 16;
  for (uint_fast32_t i = 0; i < indexStride; i++) {
    for (uint32_t j = 0; j < 16; j++) {
      *ptvt = *_pinPair[index].pixels[pixindex];
      ptvt++;
      pixindex++;
    }
    result = _mm_adds_epu8(_ligthenValue128i, value);
    for (uint32_t j = 0; j < 16; j++) {
       *_pinPair[index].pixels[pixindex2]=*ptrt;
      ptrt++;
      pixindex2++;
    }
    ptvt=ptv;
    ptrt=ptr;
  }

  // for (uint_fast32_t i = 0; i < _pinPair[index].pixels.size(); i++) {
  //   *_pinPair[index].pixels[i] += _ligthenValue;
  //   if (*_pinPair[index].pixels[i] < _ligthenValue)
  //     *_pinPair[index].pixels[i] = 255;
  // }
}
void WeaveImage::printPinSequence(void) {
  for (uint_fast32_t i = 0; i < _pinSequence.size(); i++) {
    std::cout << "Pin " << _pinSequence[i].first << " "
              << _pinSequence[i].second << std::endl;
  }
}
void WeaveImage::saveFileCallBack(std::string file) {
  std::ofstream myfile;
  std::cout << "Writng file.\n";
  myfile.open(file.c_str());

  for (uint_fast32_t i = 0; i < _pinSequence.size(); i++) {
    myfile << _pinSequence[i].first << " " << _pinSequence[i].second
           << std::endl;
  }
  myfile.close();
}
void WeaveImage::saveBinFileCallBack(std::string file) {
  std::ofstream myfile;
  std::cout << "Writng file.\n";
  myfile.open(file.c_str(), std::ios::out | std::ios::binary);

  for (uint_fast32_t i = 0; i < _pinSequence.size(); i++) {
    myfile << (uint16_t)_pinSequence[i].first;
  }
  myfile.close();
}
void WeaveImage::openNewFileCallBack(std::string file) {
  _file = file;
  SDL_FreeSurface(_surface);
  SDL_DestroyTexture(_texture);
  _surface = IMG_Load(_file.c_str());
  _texture = IMG_LoadTexture(_renderer, _file.c_str());
  SDL_QueryTexture(_texture, &_pixelFormat.format, nullptr, &_textRect.w,
                   &_textRect.h);
  _windowScale = (float)_winWidth / (float)_surface->w;
  _greyOri.clear();
  _greyWorkImg.clear();
  _generateGreyVector();
  _pinSequence.clear();
  _pinThreadCount.clear();
  _pinThreadCount.resize(_numPins, 0);
  _pinPos.clear();
  _pinPair.clear();
  _pinStride = _numPins - 2 * _pinOffset;
  _generatePinPosition();
  _generatePinPairs();
  _generateLinePixelsPointers();
  _getPinSequence();
}

// // THE EXTREMELY FAST LINE ALGORITHM Variation E (Addition Fixed Point
// PreCalc) vector<SDL_Point> WeaveImage::_getLinePixelF(int x, int y, int x2,
// int y2) {
//   SDL_Point point;
//   vector<SDL_Point> result;
//

//     if (abs(shortLen) > abs(longLen)) {
//     int swap = shortLen;
//     shortLen = longLen;
//     longLen = swap;
//     yLonger = true;
//   }
//   int decInc;
//   if (longLen == 0)
//     decInc = 0;
//   else
//     decInc = (shortLen << 16) / longLen;
//   if (yLonger) {
//     if (longLen > 0) {
//       longLen += y;
//       for (int j = 0x8000 + (x << 16); y <= longLen; ++y) {
//         point.x = j >> 16;
//         point.y = y;
//         result.push_back(point);
//         // myPixel(surface,j >> 16,y);
//         j += decInc;
//       }
//       return result;
//     }
//     longLen += y;
//     for (int j = 0x8000 + (x << 16); y >= longLen; --y) {
//       // myPixel(surface,j >> 16,y);
//       point.x = j >> 16;
//       point.y = y;
//       result.push_back(point);
//       j -= decInc;
//     }
//     return result;
//   }

//   if (longLen > 0) {
//     longLen += x;
//     for (int j = 0x8000 + (y << 16); x <= longLen; ++x) {
//       // myPixel(surface,x,j >> 16);
//       point.x = x;
//       point.y = j >> 16;
//       result.push_back(point);
//       j += decInc;
//     }
//     return result;
//   }
//   longLen += x;
//   for (int j = 0x8000 + (y << 16); x >= longLen; --x) {
//     // myPixel(surface,x,j >> 16);
//     point.x = x;
//     point.y = j >> 16;
//     result.push_back(point);
//     j -= decInc;
//   }
//   return result;
// }
// THE EXTREMELY FAST LINE ALGORITHM Variation E (Addition Fixed PointPreCalc)
// vector<Uint8 *> WeaveImage::_getLinePixelF2Pointer(int x, int y, int x2,
//                                                    int y2) {
//   vector<Uint8 *> result;
//   bool yLonger = false;
//   int shortLen = y2 - y;
//   int longLen = x2 - x;
//   if (abs(shortLen) > abs(longLen)) {
//     int swap = shortLen;
//     shortLen = longLen;
//     longLen = swap;
//     yLonger = true;
//   }
//   int decInc;
//   if (longLen == 0)
//     decInc = 0;
//   else
//     decInc = (shortLen << 16) / longLen;

//   if (yLonger) {
//     if (longLen > 0) {
//       longLen += y;
//       for (int j = 0x8000 + (x << 16); y <= longLen; ++y) {
//         result.push_back(&_greyWorkImg[j >> 16][y]);
//         j += decInc;
//       }
//       return result;
//     }
//     longLen += y;
//     for (int j = 0x8000 + (x << 16); y >= longLen; --y) {
//       result.push_back(&_greyWorkImg[j >> 16][y]);
//       j -= decInc;
//     }
//     return result;
//   }

//   if (longLen > 0) {
//     longLen += x;
//     for (int j = 0x8000 + (y << 16); x <= longLen; ++x) {
//       result.push_back(&_greyWorkImg[x][j >> 16]);
//       j += decInc;
//     }
//     return result;
//   }
//   longLen += x;
//   for (int j = 0x8000 + (y << 16); x >= longLen; --x) {
//     result.push_back(&_greyWorkImg[j >> 16][y]);
//     j -= decInc;
//   }
//   return result;
// }

// void WeaveImage::addTest() {
//   // __m256i t= _mm256_set1_epi8 (12);
//   // __m256i t2= _mm256_set1_epi8 (13);
//   // __m256i result2 = _mm256_adds_epi8(t, t2);

//   // __m128i bi = _mm_set1_epi8(253);
//   // __m128i bi2 = _mm_set1_epi8(3);
//   // __m128i resulti = y
//   //      __m256i bi = _mm256_set_epi8(
//   //                                32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
//   32,
//   //                                32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
//   32,
//   //                                32, 32, 32, 32, 32, 32, 32, 32, 32,
//   32);
//   // __m128i
//   //   __m256i bi2 = _mm256_set1_epi8(3);

//   //   __m256i resulti = _mm256_add_epi8(bi, bi2);
//   //__m256i resulti = _mm256_subs_epi8(bi, bi2);
//   /* Initialize the two argument vectors */
//   __m256 evens = _mm256_set_ps(2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0);
//   __m256 odds = _mm256_set_ps(1.0, 3.0, 5.0, 7.0, 9.0, 11.0, 13.0, 15.0);

//   /* Compute the difference between the two vectors */
//   __m256 result = _mm256_sub_ps(evens, odds);

//   /* Display the elements of the result vector */
//   float *f = (float *)&result;
//   printf("%f %f %f %f %f %f %f %f\n", f[0], f[1], f[2], f[3], f[4], f[5],
//   f[6],
//          f[7]);
//   // uint8_t *ri = (uint8_t * )&resulti;
//   // for (uint32_t i = 0; i < 16; i++) {
//   //   printf("%d ,",ri[i]);
//   // }
// }
