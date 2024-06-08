// Fill out your copyright notice in the Description page of Project Settings.


#include "LawnMowerGameMode.h"


void ALawnMowerGameMode::UpdateGrassCut(int32 Grass)
{
	GrassCut += Grass;

	UE_LOG(LogTemp, Warning, TEXT("GrassCut: %d"), GrassCut);
}

int32 ALawnMowerGameMode::GetGrassCut() const { return GrassCut; };