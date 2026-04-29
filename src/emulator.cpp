#ifdef NATIVE

#include <chrono>
#include <thread>
#include <atomic>
#include <SDL3/SDL.h>
#include <Arduino.h>

#define private public
#define protected public
#include "draw.h"
#undef private
#undef protected

// 模拟器屏幕旋转方向：
// 0: 竖屏
// 1: 横屏(顺时针90度)
// 2: 倒置竖屏(顺时针180度)
// 3: 倒置横屏(顺时针270度)
#ifndef EMULATOR_ROTATION
#define EMULATOR_ROTATION 0
#endif

// 模拟器屏幕窗口的放大倍数
#ifndef EMULATOR_SCALE
#define EMULATOR_SCALE 2
#endif

// 模拟器屏幕刷新率
#ifndef EMULATOR_FPS
#define EMULATOR_FPS 20
#endif

extern EPD_CLASS epd;
extern bool keyPressed;
extern void refreshPage();

template<typename GxEPD2_Class>
uint32_t get_color(GxEPD2_Class& epd, int x, int y);

template<typename GxEPD2_Type, const uint16_t page_height>
uint32_t get_color(GxEPD2_BW<GxEPD2_Type, page_height>& epd, int x, int y) {
    int byte_idx = (x / 8) + y * (epd.WIDTH / 8);
    uint8_t bit_mask = 1 << (7 - (x % 8));

    bool is_black = !(epd._buffer[byte_idx] & bit_mask);

    uint32_t color = 0xFFFFFFFF;
    if (is_black) color = 0xFF000000;      // 黑
    return color;
}

template<typename GxEPD2_Type, const uint16_t page_height>
uint32_t get_color(GxEPD2_3C<GxEPD2_Type, page_height>& epd, int x, int y) {
    int byte_idx = (x / 8) + y * (epd.WIDTH / 8);
    uint8_t bit_mask = 1 << (7 - (x % 8));

    bool is_black = !(epd._black_buffer[byte_idx] & bit_mask);
    bool is_color = !(epd._color_buffer[byte_idx] & bit_mask);

    uint32_t color = 0xFFFFFFFF;
    if (is_black) color = 0xFF000000;      // 黑
    else if (is_color) color = 0xFFFF0000; // 红/黄
    return color;
}

int main(int argc, char* argv[]) {
    // 使日志能在 pio 的输出窗口上实时打印
    setbuf(stdout, NULL);

    // 墨水屏尺寸
    int width = epd.WIDTH;
    int height = epd.HEIGHT;
    // SDL 窗口大小
    const int rotation = EMULATOR_ROTATION % 4;
    int win_width = (rotation % 2 == 1) ? height : width;
    int win_height = (rotation % 2 == 1) ? width : height;

    // 初始化 SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return -1;
    }
    SDL_Window* window = SDL_CreateWindow("EInkAssistant Emulator | F2 switch page | F5 refresh page",
                                            win_width * EMULATOR_SCALE, win_height * EMULATOR_SCALE, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, win_width, win_height);
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_PIXELART);

    // 启动 Arduino 线程
    std::atomic<bool> running{true};
    std::atomic<bool> refresh{false};
    std::thread arduino_thread([&running, &refresh]() {
        setup();
        while (running) {
            loop();
            if (refresh) {
                refreshPage();
                refresh = false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    // SDL 主循环
    uint32_t* pixels = new uint32_t[win_width * win_height];
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            } else if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_F2) {
                    keyPressed = true;
                } else if (event.key.key == SDLK_F5) {
                    refresh = true;
                }
            }
        }

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int win_x = x;
                int win_y = y;
                if (rotation == 1) {
                    win_x = height - 1 - y;
                    win_y = x;
                } else if (rotation == 2) {
                    win_x = width - 1 - x;
                    win_y = height - 1 - y;
                } else if (rotation == 3) {
                    win_x = y;
                    win_y = width - 1 - x;
                }
                pixels[win_y * win_width + win_x] = get_color(epd, x, y);
            }
        }

        SDL_UpdateTexture(texture, nullptr, pixels, win_width * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        SDL_Delay(1000 / EMULATOR_FPS);
    }

    // 等待 Arduino 线程退出
    arduino_thread.join();

    // 释放资源
    delete[] pixels;
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
#endif
