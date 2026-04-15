#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "ItemProxy_Container.h"
#include "InventoryComponent.generated.h"

class FLifetimeProperty;
class UItemDefine;

struct FBasicProxy;
struct FProxyStrategy;

/**
 * 库存组件（服务端权威）：
 * 1) 维护运行时 Proxy 列表（ProxysAry）
 * 2) 维护 FastArray 容器（Proxy_FASI_Container）用于网络复制
 * 3) 通过 ItemTag 将 Proxy 关联到 UItemDefine 原型
 *
 * 设计约束：
 * - 客户端不直接改库存，只能请求服务端 RPC
 * - 服务端改动后必须同步修改 FastArray（Add/Update/Remove）
 * - 客户端通过 FastArray 回调增量更新本地缓存
 */
UCLASS()
class INVENTORYSYSTEM_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps
		) const override;

	// 增加指定类型道具数量（服务端执行；客户端会转发 RPC）
	TWeakPtr<FBasicProxy> AddProxy(
		const FGameplayTag& ProxyType,
		uint8 Num
		);

	// 移除指定类型道具数量（服务端执行；客户端会转发 RPC）
	bool RemoveProxy(
		const FGameplayTag& ProxyType,
		uint8 Num
		);

	// 查询某类型道具总数（汇总堆叠）
	int32 GetProxyCount(
		const FGameplayTag& ProxyType
		) const;

	// 获取当前全部运行时 Proxy
	const TArray<TSharedPtr<FBasicProxy>>& GetAllProxyList() const;

	// 注册 Proxy 构造策略（按 Tag 创建子类 Proxy）
	void AddGetProxyMetaStrategy(
		const TSharedPtr<FProxyStrategy>& ProxyMetaStrategyFunc
		);

	// 客户端请求：新增道具
	UFUNCTION(Server, Reliable)
	void AddProxy_Server(
		const FGameplayTag& ProxyType,
		uint8 Num
		);

	// 客户端请求：移除道具
	UFUNCTION(Server, Reliable)
	void RemoveProxy_Server(
		const FGameplayTag& ProxyType,
		uint8 Num
		);

protected:
	// 复制兜底回调：用于全量重建缓存，避免增量事件遗漏导致漂移
	UFUNCTION()
	void OnRep_ProxyContainer();

	// FastArray 事件：新增
	void HandleProxyAdded(
		const TSharedPtr<FBasicProxy>& ProxySPtr
		);

	// FastArray 事件：变更
	void HandleProxyChanged(
		const TSharedPtr<FBasicProxy>& ProxySPtr
		);

	// FastArray 事件：移除
	void HandleProxyRemoved(
		const TSharedPtr<FBasicProxy>& ProxySPtr
		);

	// 根据 Tag 寻找最合适的 Proxy 构造策略（精确匹配优先，其次父标签匹配）
	TSharedPtr<FProxyStrategy> FindProxyMetaStrategy(
		const FGameplayTag& ProxyType
		) const;

	// 基于 ItemDefine 构造一个运行时 Proxy（可能是具体子类）
	TSharedPtr<FBasicProxy> CreateProxyInstance(
		const UItemDefine* ItemDefine
		) const;

	// 给 Proxy 回填 ItemDefine 指针（客户端反序列化后通常需要补）
	void SyncProxyDefine(
		const TSharedPtr<FBasicProxy>& ProxySPtr
		);

	// 直接按 FastArray 当前状态全量重建缓存
	void RefreshProxyCacheFromContainer();

protected:
	// Tag -> Proxy 构造策略
	TMap<FGameplayTag, TSharedPtr<FProxyStrategy>> GetProxyMetaStrategies;

	// 已加载的原型数据缓存（Tag -> UItemDefine）
	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UItemDefine>> AllItemDefineMap;

	// 运行时缓存（不做 UPROPERTY 反射，TSharedPtr 不受 UHT 支持）
	TArray<TSharedPtr<FBasicProxy>> ProxysAry;

	// 网络复制容器：服务端写，客户端收
	UPROPERTY(ReplicatedUsing = OnRep_ProxyContainer)
	FProxy_FASI_Container Proxy_FASI_Container;
};
