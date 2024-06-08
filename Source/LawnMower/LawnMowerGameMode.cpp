// Fill out your copyright notice in the Description page of Project Settings.


#include "LawnMowerGameMode.h"


void ALawnMowerGameMode::UpdateGrassCut(int32 Grass) { GrassCut += Grass; }

int32 ALawnMowerGameMode::GetGrassCut() const { return GrassCut; };