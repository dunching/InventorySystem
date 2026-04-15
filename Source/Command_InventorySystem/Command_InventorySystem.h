// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

namespace GameplayCommand
{
	// 控制台命令：AddProxy <GameplayTag>
	void AddProxy(
		const TArray<FString>& Args
		);

	static FAutoConsoleCommand AddProxyCMD(
		TEXT("AddProxy"),
		TEXT("按 GameplayTag 添加库存条目。"),
		FConsoleCommandWithArgsDelegate::CreateStatic(AddProxy),
		EConsoleVariableFlags::ECVF_Default
	);
}
