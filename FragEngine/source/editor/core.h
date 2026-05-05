#pragma once

#include "core/Fragment.h"

inline Fragment* editorEngine;

inline Graphics* editorGraphics;
inline Window* editorWindow;
inline Camera* editorCamera;
inline UI* editorUI;
inline Input* editorInput;
inline LightSource* editorLight;

inline Style editorUIStyle;

void startEditor();

void updateEditor(float dt);

void drawEditor();

void endEditor();

void setupUI();

void start();