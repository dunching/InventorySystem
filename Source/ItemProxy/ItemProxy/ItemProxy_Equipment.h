#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "ItemProxy.h"
#include "ItemProxy_Equipment.generated.h"

/**
 * 装备实例 Proxy：
 * - 当前不追加专属字段
 * - 复用 FBasicProxy 的基础复制能力（ProxyId/ItemTag/Count/ValuesMap）
 * - 后续可在此扩展“耐久、品阶、词条”等装备专有属性
 */
USTRUCT()
struct ITEMPROXY_API FItemProxy_Equipment : public FBasicProxy
{
	GENERATED_USTRUCT_BODY()

public:
	virtual bool NetSerialize(
		FArchive& Ar,
		class UPackageMap* Map,
		bool& bOutSuccess
		) override;
};

template <>
struct TStructOpsTypeTraits<FItemProxy_Equipment> : public TStructOpsTypeTraitsBase2<FItemProxy_Equipment>
{
	enum
	{
		WithNetSerializer = true,
	};
};

/**
 * 装备 Proxy 构造策略：
 * - ItemTag 命中装备标签时，创建 FItemProxy_Equipment
 */
struct ITEMPROXY_API FProxy_EquipmentStrategy : public FProxyStrategy
{
	virtual FGameplayTag GetTag() const override;

	virtual TSharedPtr<FBasicProxy> GetProxy() const override;
};
