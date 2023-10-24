#include <math.h>
#include <stdlib.h>

#include "main.h"
#include "pros/screen.h"
#include "pros/screen.hpp"

#ifndef PI
#define PI 3.14159265358979323846f
#endif
#ifndef DEG2RAD
#define DEG2RAD (PI / 180.0f)
#endif
#ifndef RAD2DEG
#define RAD2DEG (180.0f / PI)
#endif

int constexpr const PADDLE_H = 50;
int constexpr const PADDLE_W = 5;
int constexpr const SCREEN_W = 480;
int constexpr const SCREEN_H = 272 - 32;
int constexpr const BALL_R = 5;
float constexpr const PADDLE_SPEED = 0.1;
float constexpr const BALL_SPEED = 4;

template <typename val> val clamp(val n, val min, val max) {
  if (n < min)
    n = min;
  if (n > max)
    n = max;
  return n;
}

int between(int x, int minx, int maxx) {
  return ((x - minx) | (maxx - x)) >= 0;
}

void initialize() {}

void disabled() {}
void competition_initialize() {}
void autonomous() {}

int position_pad1 = 0;
int position_pad2 = 0;

struct Vector2 {
  float x, y;
};

Vector2 ball, ball_vel;
int ball_speed = BALL_SPEED;
float ball_dir = 0;

float random_float(float a, float b) {
  float random = ((float)rand()) / (float)RAND_MAX;
  float diff = b - a;
  float r = random * diff;
  return a + r;
}

int score_p1 = 0, score_p2 = 0;

void reset_game(bool player1_won) {
  position_pad1 = SCREEN_H / 2 - PADDLE_H / 2;
  position_pad2 = SCREEN_H / 2 - PADDLE_H / 2;

  ball.x = (float)SCREEN_W / 2 - BALL_R;
  ball.y = (float)SCREEN_H / 2 - BALL_R;

  ball_vel.x = 1;
  ball_vel.y = 1;

  ball_speed = BALL_SPEED;

  if (player1_won) {
    ball_dir = random_float(-45 * DEG2RAD, 45 * DEG2RAD);
    score_p1++;
  } else {
    ball_dir = random_float(135 * DEG2RAD, 225 * DEG2RAD);
    score_p2++;
  }
}

void increase_ball_speed() { ball_speed *= 1.5; }

void opcontrol() {
  pros::Controller master(pros::E_CONTROLLER_MASTER);

  reset_game(true);
  score_p1 = score_p2 = 0;

  while (true) {
    int direction_p1 = master.get_analog(ANALOG_LEFT_Y);
    position_pad1 -= direction_p1 * PADDLE_SPEED;
    position_pad1 = clamp(position_pad1, PADDLE_H / 2, SCREEN_H - PADDLE_H);

    int direction_p2 = master.get_analog(ANALOG_RIGHT_Y);
    position_pad2 -= direction_p2 * PADDLE_SPEED;
    position_pad2 = clamp(position_pad2, PADDLE_H / 2, SCREEN_H - PADDLE_H);

    int new_ball_pos_x, new_ball_pos_y;
    new_ball_pos_x = ball.x + ball_vel.x * BALL_SPEED;
    new_ball_pos_y = ball.y + ball_vel.y * BALL_SPEED;

    if (new_ball_pos_y + BALL_R > SCREEN_H || new_ball_pos_y - BALL_R < 0) {
      ball_dir = 2 * PI - ball_dir;
      ball_vel.y = -sin(ball_dir);
      ball.y += ball_vel.y;
      increase_ball_speed();
    }

    if (new_ball_pos_x >= SCREEN_W - PADDLE_W - BALL_R &&
        between(new_ball_pos_y, position_pad2 - BALL_R,
                position_pad2 + PADDLE_H + BALL_R)) {
      float relative_intersect_y = (position_pad2 + (PADDLE_H / 2.)) - ball.y;
      float normalized_relative_intersection_y =
          relative_intersect_y / (PADDLE_H / 2.);
      ball_dir = PI - normalized_relative_intersection_y * 75 * DEG2RAD;

      increase_ball_speed();
    }

    if (new_ball_pos_x <= PADDLE_W + BALL_R && new_ball_pos_x > 0 &&
        between(new_ball_pos_y, position_pad1 - BALL_R,
                position_pad1 + PADDLE_H + BALL_R)) {
      float relative_intersect_y = (position_pad1 + (PADDLE_H / 2.)) - ball.y;
      float normalized_relative_intersection_y =
          relative_intersect_y / (PADDLE_H / 2.);
      ball_dir = normalized_relative_intersection_y * 75 * DEG2RAD;

      increase_ball_speed();
    }

    ball_vel.x = cos(ball_dir);
    ball_vel.y = -sin(ball_dir);

    ball.x += ball_vel.x * BALL_SPEED;
    ball.y += ball_vel.y * BALL_SPEED;

    if (ball.x > SCREEN_W) {
      reset_game(true);
      continue;
    } else if (ball.x < 0) {
      reset_game(false);
      continue;
    }

    pros::screen::erase();
    pros::screen::set_pen(COLOR_RED);
    pros::screen::fill_rect(0, position_pad1 - PADDLE_H / 2, PADDLE_W,
                            position_pad1 + PADDLE_H);
    pros::screen::set_pen(COLOR_BLUE);
    pros::screen::fill_rect(SCREEN_W - PADDLE_W, position_pad2 - PADDLE_H / 2,
                            SCREEN_W, position_pad2 + PADDLE_H);

    pros::screen::set_pen(0x00333333);
    pros::screen::fill_rect(SCREEN_W / 2 - 1, 0, SCREEN_W / 2 + 1, SCREEN_H);

    pros::screen::set_pen(COLOR_WHITE);
    pros::screen::fill_circle(ball.x, ball.y, BALL_R);

    pros::screen::print(pros::E_TEXT_LARGE_CENTER, 2, 2, "%d", score_p1);
    pros::screen::print(pros::E_TEXT_LARGE, SCREEN_W / 2 + 1 + 2, 2, "%d",
                        score_p2);

    pros::delay(20);
  }
}
