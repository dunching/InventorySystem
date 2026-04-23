#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ItemProxy.h"
#include "ItemProxy_Modifier.generated.h"

/**
 * 强化/词条实例 Proxy：
 * - 当前不追加专属字段
 * - 统一使用 FBasicProxy 的基础字段和 ValuesMap
 * - 后续可在此扩展“层数、充能、触发计数”等强化专属状态
 */
USTRUCT()
struct ITEMPROXY_API FItemProxy_Modifier : public FBasicProxy
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
struct TStructOpsTypeTraits<FItemProxy_Modifier> : public TStructOpsTypeTraitsBase2<FItemProxy_Modifier>
{
	enum
	{
		WithNetSerializer = true,
	};
};

/**
 * 强化 Proxy 构造策略：
 * - ItemTag 命中强化标签时，创建 FItemProxy_Modifier
 */
struct ITEMPROXY_API FProxy_ModifierStrategy : public FProxyStrategy
{
	virtual FGameplayTag GetTag() const override;

	virtual TSharedPtr<FBasicProxy> GetProxy() const override;
};
