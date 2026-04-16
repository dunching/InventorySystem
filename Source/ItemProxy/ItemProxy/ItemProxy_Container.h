#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "ItemProxy_Container.generated.h"

struct FBasicProxy;

/**
 * FastArray 单条目包装：
 * - 持有一个运行时 Proxy 指针（FBasicProxy）
 * - 负责 Proxy 的网络序列化
 * - 在客户端复制生命周期回调中派发事件
 */
USTRUCT()
struct ITEMPROXY_API FProxy_FASI : public FFastArraySerializerItem
{
	GENERATED_USTRUCT_BODY()

	// 客户端：条目被移除前触发
	void PreReplicatedRemove(
		const struct FProxy_FASI_Container& InArraySerializer
		);

	// 客户端：条目新增后触发
	void PostReplicatedAdd(
		const struct FProxy_FASI_Container& InArraySerializer
		);

	// 客户端：条目变更后触发
	void PostReplicatedChange(
		const struct FProxy_FASI_Container& InArraySerializer
		);

	// 核心网络序列化：将 Proxy 数据写入/读出网络包
	bool NetSerialize(
		FArchive& Ar,
		class UPackageMap* Map,
		bool& bOutSuccess
		);

	// FastArray 用于比较条目身份（按 ProxyId）
	bool operator==(
		const FProxy_FASI& Right
		) const;

	// 运行时实例指针（服务端维护，客户端复制后重建）
	TSharedPtr<FBasicProxy> ProxySPtr = nullptr;
};

template <>
struct TStructOpsTypeTraits<FProxy_FASI> : public TStructOpsTypeTraitsBase2<FProxy_FASI>
{
	enum
	{
		WithNetSerializer = true,
	};
};

/**
 * FastArray 容器：
 * - 服务端通过 Add/Update/Remove 驱动增量复制
 * - 客户端通过 OnAdd/OnChange/OnRemove 监听差量更新
 */
USTRUCT()
struct ITEMPROXY_API FProxy_FASI_Container : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	using FItemType = FProxy_FASI;
	using FContainerType = FProxy_FASI_Container;
	using FOnChanged = TMulticastDelegate<void(const TSharedPtr<FBasicProxy>&)>;

	// FastArray 实际条目列表（会参与复制）
	UPROPERTY()
	TArray<FProxy_FASI> Items;

	// FastArray Delta 复制入口
	bool NetDeltaSerialize(
		FNetDeltaSerializeInfo& DeltaParms
		);

	// FastArray 模板回调：客户端写包时只写有效复制 ID 的条目
	template <typename Type, typename SerializerType>
	bool ShouldWriteFastArrayItem(
		const Type& Item,
		const bool bIsWritingOnClient
		)
	{
		if (bIsWritingOnClient)
		{
			return Item.ReplicationID != INDEX_NONE;
		}

		return true;
	}

	// 服务端新增一条 Proxy
	void AddItem(
		const TSharedPtr<FBasicProxy>& ProxySPtr
		);

	// 服务端按 ProxyId 标记一条 Proxy 变更
	void UpdateItem(
		const FGuid& ProxyId
		);

	// 服务端按 Proxy 指针标记一条 Proxy 变更
	void UpdateItem(
		const TSharedPtr<FBasicProxy>& ProxySPtr
		);

	// 服务端移除一条 Proxy
	void RemoveItem(
		const TSharedPtr<FBasicProxy>& ProxySPtr
		);

	// 客户端：新增事件（PostReplicatedAdd）
	FOnChanged OnAdd;

	// 客户端：变更事件（PostReplicatedChange）
	FOnChanged OnChange;

	// 客户端：移除事件（PreReplicatedRemove）
	FOnChanged OnRemove;
};

template <>
struct TStructOpsTypeTraits<FProxy_FASI_Container> : public TStructOpsTypeTraitsBase2<FProxy_FASI_Container>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
