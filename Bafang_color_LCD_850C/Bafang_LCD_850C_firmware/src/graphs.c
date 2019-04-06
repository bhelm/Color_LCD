/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018, 2019.
 *
 * Released under the GPL License, Version 3
 */

//#include <math.h>
#include "graphs.h"

#include <string.h>
#include "stm32f10x.h"
#include "stdio.h"
#include "main.h"
#include "config.h"
#include "ugui_driver/ugui_bafang_500c.h"
#include "ugui/ugui.h"
#include "lcd.h"

uint32_t ui32_array_data[255 * 4] =
{
    239 ,
    241 ,
    242 ,
    243 ,
    245 ,
    246 ,
    247 ,
    248 ,
    249 ,
    250 ,
    251 ,
    251 ,
    252 ,
    253 ,
    253 ,
    254 ,
    254 ,
    254 ,
    255 ,
    255 ,
    255 ,
    255 ,
    255 ,
    255 ,
    254 ,
    254 ,
    254 ,
    253 ,
    253 ,
    252 ,
    251 ,
    250 ,
    250 ,
    249 ,
    248 ,
    247 ,
    245 ,
    244 ,
    243 ,
    242 ,
    240 ,
    239 ,
    236 ,
    231 ,
    227 ,
    222 ,
    217 ,
    212 ,
    207 ,
    202 ,
    197 ,
    191 ,
    186 ,
    181 ,
    176 ,
    170 ,
    165 ,
    160 ,
    154 ,
    149 ,
    144 ,
    138 ,
    133 ,
    127 ,
    122 ,
    116 ,
    111 ,
    106 ,
    100 ,
    95  ,
    89  ,
    84  ,
    79  ,
    74  ,
    68  ,
    63  ,
    58  ,
    53  ,
    48  ,
    43  ,
    38  ,
    33  ,
    28  ,
    23  ,
    18  ,
    16  ,
    14  ,
    13  ,
    12  ,
    10  ,
    9 ,
    8 ,
    7 ,
    6 ,
    5 ,
    4 ,
    3 ,
    3 ,
    2 ,
    1 ,
    1 ,
    1 ,
    0 ,
    0 ,
    0 ,
    0 ,
    0 ,
    0 ,
    0 ,
    0 ,
    0 ,
    1 ,
    1 ,
    2 ,
    2 ,
    3 ,
    4 ,
    5 ,
    6 ,
    6 ,
    8 ,
    9 ,
    10  ,
    11  ,
    12  ,
    14  ,
    15  ,
    17  ,
    15  ,
    14  ,
    12  ,
    11  ,
    10  ,
    9 ,
    8 ,
    6 ,
    6 ,
    5 ,
    4 ,
    3 ,
    2 ,
    2 ,
    1 ,
    1 ,
    0 ,
    0 ,
    0 ,
    0 ,
    0 ,
    0 ,
    0 ,
    0 ,
    0 ,
    1 ,
    1 ,
    1 ,
    2 ,
    3 ,
    3 ,
    4 ,
    5 ,
    6 ,
    7 ,
    8 ,
    9 ,
    10  ,
    12  ,
    13  ,
    14  ,
    16  ,
    18  ,
    23  ,
    28  ,
    33  ,
    38  ,
    43  ,
    48  ,
    53  ,
    58  ,
    63  ,
    68  ,
    74  ,
    79  ,
    84  ,
    89  ,
    95  ,
    100 ,
    106 ,
    111 ,
    116 ,
    122 ,
    127 ,
    133 ,
    138 ,
    144 ,
    149 ,
    154 ,
    160 ,
    165 ,
    170 ,
    176 ,
    181 ,
    186 ,
    191 ,
    197 ,
    202 ,
    207 ,
    212 ,
    217 ,
    222 ,
    227 ,
    231 ,
    236 ,
    239 ,
    240 ,
    242 ,
    243 ,
    244 ,
    245 ,
    247 ,
    248 ,
    249 ,
    250 ,
    250 ,
    251 ,
    252 ,
    253 ,
    253 ,
    254 ,
    254 ,
    254 ,
    255 ,
    255 ,
    255 ,
    255 ,
    255 ,
    255 ,
    254 ,
    254 ,
    254 ,
    253 ,
    253 ,
    252 ,
    251 ,
    251 ,
    250 ,
    249 ,
    248 ,
    247 ,
    246 ,
    245 ,
    243 ,
    242 ,
    241 ,
    239 ,
    238 ,
};

#define GRAPH_START_X 56
#define GRAPH_START_Y (480 - 5)
#define GRAPH_Y_LENGHT 100

// 255 pixels for data points
// 255 points of each graph
// 60 minutes: 255 * (3500 * 4) ms
// 30 minutes: 255 * (3500 * 2) ms
// 15 minutes: 255 * 3500 ms

graphs_t graphs[5];

// graph ID based on array index number
typedef enum
{
  BATTERY_VOLTAGE = 0,
  BATTERY_CURRENT = 1,
  PEDAL_CADENCE = 2,
  PEDAL_TORQUE = 3,
  PEDAL_HUMAN_POWER = 4
} graph_id_t;

// every 3500ms
void graphs_draw(void)
{
  uint16_t i;
  uint32_t number_lines_to_draw;
  uint32_t y_amplitude;
  uint32_t graph_next_start_x;

  graphs_update_data();

  number_lines_to_draw = graphs[0].ui32_data_last_index - graphs[0].ui32_data_start_index;
  graph_next_start_x = GRAPH_START_X + graphs[0].ui32_x_last_index + 1;

  // draw only the next line and keep the others intact
  for(i = 0; i < number_lines_to_draw; i++)
  {
    y_amplitude = graphs[0].ui32_data[graphs[0].ui32_x_last_index + i] - graphs[0].ui32_graph_data_y_min;
    y_amplitude *= graphs[0].ui32_data_y_rate_per_pixel_x100;
    y_amplitude /= 100;

    UG_DrawLine(graph_next_start_x + i,       // X1
                GRAPH_START_Y,                // Y1
                graph_next_start_x + i,       // X2
                GRAPH_START_Y - y_amplitude,  //Y2
                C_WHITE);
  }
}


// every 3500ms
void graphs_update_data(void)
{
  UG_FillFrame(GRAPH_START_X, GRAPH_START_Y - 100, 315, GRAPH_START_Y, C_BLACK);

  memcpy(graphs[0].ui32_data, ui32_array_data, (255 * 4) * 4);

  graphs[0].ui32_graph_data_y_min = 0;
  graphs[0].ui32_graph_data_y_max = 255;
  graphs[0].ui32_data_y_rate_per_pixel_x100 = (GRAPH_Y_LENGHT * 100) /
      (graphs[0].ui32_graph_data_y_max - graphs[0].ui32_graph_data_y_min);
  graphs[0].ui32_data_last_index = 254;
  graphs[0].ui32_data_start_index = 0;
}

void graphs_init(void)
{
  memcpy(graphs[0].ui32_data, ui32_array_data, (255 * 4) * 4);
}

graphs_t *get_graphs(void)
{
  return graphs;
}
