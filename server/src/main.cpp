/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
*/

#include "bus/cantomqttapp.h"

int main(int argc, char **argv) {
  bus::CanToMqttApp app;
  return app.MainFunc(argc,argv);
}