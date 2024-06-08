// Fill out your copyright notice in the Description page of Project Settings.


#include "LawnMowerGameMode.h"


void ALawnMowerGameMode::UpdateGrassCut() { ++GrassCut; }

int32 ALawnMowerGameMode::GetGrassCut() const { return GrassCut; };