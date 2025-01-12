#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "esp_lvgl_port.h"
#include "lvgl.h"

#include "esp_lcd_panel_vendor.h"

#include <string>
#include <algorithm>

using namespace std;

static const char *TAG = "graphic";

// TODO
//  vedere https://docs.lvgl.io/master/details/widgets/scale.html

extern lv_obj_t *scr;

struct position
{
    int x;
    int y;
};

class blocco
{
public:
    int min;
    int max;
    int step;
    int current;
    position pos;
    lv_obj_t *nome;
    lv_obj_t *valore;
    lv_style_t selectedStyle;
    lv_style_t hoveredStyle;
    // lv_style_t unselectedStyle;
    blocco(string nome, position pos, int min = 0, int max = 100, int step = 1, int current = 50)
    {
        this->min = min;
        this->max = max;
        this->step = step;
        this->current = current;
        this->pos = pos;

        this->nome = lv_label_create(scr);
        lv_label_set_text(this->nome, nome.c_str());
        lv_obj_align(this->nome, LV_ALIGN_TOP_LEFT, pos.x, pos.y);
        this->valore = lv_label_create(scr);
        lv_label_set_text_fmt(this->valore, "%d", current);
        lv_obj_align(this->valore, LV_ALIGN_TOP_LEFT, pos.x, pos.y + 20);

        lv_style_init(&selectedStyle);
        lv_style_set_text_decor(&selectedStyle, LV_TEXT_DECOR_STRIKETHROUGH);

        lv_style_init(&hoveredStyle);
        //for hovered do underline
        lv_style_set_text_decor(&hoveredStyle, LV_TEXT_DECOR_UNDERLINE);

        ESP_LOGI(TAG, "Finished creating blocco");
    }
    void rotate(int rotation)
    {
        ESP_LOGI(TAG, "Started rotating blocco");
        this->current += rotation * this->step;
        this->current = std::max(this->min, this->current);
        this->current = std::min(this->max, this->current);
        this->updateValore();
        ESP_LOGI(TAG, "Finished rotating blocco");
    }
    void select()
    {
        lv_obj_add_style(this->nome, &selectedStyle, LV_PART_MAIN);
        lv_obj_add_style(this->valore, &selectedStyle, LV_PART_MAIN); // LV_PART_MAIN ???
    }

    void hover()
    {
        lv_obj_add_style(this->nome, &hoveredStyle, LV_PART_MAIN);
        lv_obj_add_style(this->valore, &hoveredStyle, LV_PART_MAIN);
    }

    void unhover()
    {
        lv_obj_remove_style(this->nome, &hoveredStyle, LV_PART_MAIN);
        lv_obj_remove_style(this->valore, &hoveredStyle, LV_PART_MAIN);
    }

    void unselect()
    {
        lv_obj_remove_style(this->nome, &selectedStyle, LV_PART_MAIN);
        lv_obj_remove_style(this->valore, &selectedStyle, LV_PART_MAIN);
    }
    void updateValore()
    {
        lv_label_set_text_fmt(this->valore, "%d", this->current);
    }
    ~blocco()
    {
    }
};

class screen
{
};

class adsr : public screen
{
public:
    int numBlocchi = 4;
    blocco blocchi[4] = {
        blocco("A", {30, 20}),
        blocco("D", {50, 20}),
        blocco("S", {70, 20}),
        blocco("R", {90, 20})};
    bool selected = false;
    int hoveredBlocco = 0;
    adsr()
    {
        blocchi[0].hover();
    }
    void push()
    {
        if (selected)
        {
            blocchi[hoveredBlocco].unselect();
            blocchi[hoveredBlocco].hover();
        }
        else
        {
            blocchi[hoveredBlocco].unhover();
            blocchi[hoveredBlocco].select();
        }
        selected = !selected;
    }
    void longPush()
    {
    }
    void rotate(int rotation)
    {
        if (selected)
        {
            blocchi[hoveredBlocco].rotate(rotation);
        }
        else
        {
            blocchi[hoveredBlocco].unhover();
            hoveredBlocco += rotation;
            hoveredBlocco = std::max(0, hoveredBlocco);
            hoveredBlocco = std::min(hoveredBlocco, numBlocchi - 1);
            ESP_LOGI(TAG, "%d", hoveredBlocco);
            blocchi[hoveredBlocco].hover();
        }
    }
    void update()
    {
    }
    void draw()
    {
    }
};

adsr *a;

extern "C" void getEncoderValue(int *ret_val, bool *pressed);

extern "C" void startGraphic()
{
    ESP_LOGI(TAG, "Starting graphic");
    a = new adsr();
}

extern "C" void runGraphic()
{


        int rotation;
        bool pressed;
        getEncoderValue(&rotation, &pressed);

        lvgl_port_lock(0);
        if (pressed)
        {
            ESP_LOGI(TAG, "Pressing");
            a->push();
        }
        else if (rotation != 0)
        {
            ESP_LOGI(TAG, "Rotating");
            a->rotate(rotation);
        }
        lvgl_port_unlock();
}