#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ItemProxy.h"
#include "ItemProxy_State.generated.h"

/**
 * 通用状态复制 Proxy：
 * - 不定义任何专用字段
 * - 所有状态都通过 FBasicProxy::ValuesMap(Tag->float) 表达
 * - 适用于武器状态、平台强化状态、武器强化状态等
 */
USTRUCT()
struct ITEMPROXY_API FItemProxy_State : public FBasicProxy
{
	GENERATED_USTRUCT_BODY()

public:
	virtual bool NetSerialize(
		FArchive& Ar,
		class UPackageMap* Map,
		bool& bOutSuccess
		) override;

	// 设置任意状态值（Tag 驱动）
	void SetStateValue(
		const FGameplayTag& StateTag,
		float Value
		);

	// 读取任意状态值；不存在时返回 DefaultValue
	float GetStateValue(
		const FGameplayTag& StateTag,
		float DefaultValue = 0.0f
		) const;

	// 设置布尔状态（0/1）
	void SetStateBool(
		const FGameplayTag& StateTag,
		bool bValue
		);

	// 读取布尔状态（>0 视为 true）
	bool GetStateBool(
		const FGameplayTag& StateTag
		) const;
};

template <>
struct TStructOpsTypeTraits<FItemProxy_State> : public TStructOpsTypeTraitsBase2<FItemProxy_State>
{
	enum
	{
		WithNetSerializer = true,
	};
};

/**
 * 通用状态 Proxy 构造策略：
 * - ItemTag 命中 Item.State（或其子标签）时创建该 Proxy
 */
struct ITEMPROXY_API FProxy_StateStrategy : public FProxyStrategy
{
	virtual FGameplayTag GetTag() const override;

	virtual TSharedPtr<FBasicProxy> GetProxy() const override;
};
