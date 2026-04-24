#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ItemProxy_Container.h"
#include "InventoryComponent.generated.h"

class UItemDefine;
class FLifetimeProperty;

struct FBasicProxy;
struct FProxyStrategy;
DECLARE_MULTICAST_DELEGATE(FOnInventoryProxyStateChanged);

/**
 * 库存组件：
 * - 维护运行时代理条目
 * - 通过 ItemTag 解析道具数据资产
 * - 基于 FastArray 复制库存变更
 */
UCLASS()
class INVENTORYSYSTEM_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps
		) const override;

	TWeakPtr<FBasicProxy> AddProxy(
		const FGameplayTag& ProxyType,
		uint8 Num
		);

	// 在权威端移除数量；客户端会通过 RPC 转发请求。
	bool RemoveProxy(
		const FGameplayTag& ProxyType,
		uint8 Num
		);

	int32 GetProxyCount(
		const FGameplayTag& ProxyType
		) const;

	const TArray<TSharedPtr<FBasicProxy>>& GetAllProxyList() const;

	FOnInventoryProxyStateChanged& OnInventoryProxyStateChanged()
	{
		return InventoryProxyStateChangedEvent;
	}

	UFUNCTION(BlueprintPure, Category = "Inventory")
	const UItemDefine* FindItemDefineByTag(const FGameplayTag& ItemTag) const;

	void AddGetProxyMetaStrategy(
		const TSharedPtr<FProxyStrategy>& ProxyMetaStrategyFunc
		);

	UFUNCTION(Server, Reliable)
	void AddProxy_Server(
		const FGameplayTag& ProxyType,
		uint8 Num
		);

	UFUNCTION(Server, Reliable)
	void RemoveProxy_Server(
		const FGameplayTag& ProxyType,
		uint8 Num
		);

protected:
	UFUNCTION()
	void OnRep_ProxyContainer();
	
	void HandleProxyAdded(
		const TSharedPtr<FBasicProxy>& ProxySPtr
		);

	void HandleProxyChanged(
		const TSharedPtr<FBasicProxy>& ProxySPtr
		);
	void HandleProxyRemoved(
		const TSharedPtr<FBasicProxy>& ProxySPtr
		);

	TSharedPtr<FProxyStrategy> FindProxyMetaStrategy(
		const FGameplayTag& ProxyType
		) const;

	TSharedPtr<FBasicProxy> CreateProxyInstance(
		const UItemDefine* ItemDefine
		) const;

	void SyncProxyDefine(
		const TSharedPtr<FBasicProxy>& ProxySPtr
		);
	void HandleItemDefineLoaded(const FGameplayTag& ItemTag, const UItemDefine* ItemDefine);
	void ReplayPendingAddRequests(const FGameplayTag& ItemTag);

	void RefreshProxyCacheFromContainer();
	void NotifyInventoryChanged();

protected:
	TMap<FGameplayTag, TSharedPtr<FProxyStrategy>> ProxyStrategiesBySchema;

	// 当 ItemDefine 仍在异步加载时，服务端先缓存新增请求，待资源可用后回放。
	TMap<FGameplayTag, int32> PendingAddRequests;

	// Runtime cache, not reflected: UHT does not support TSharedPtr properties.
	TArray<TSharedPtr<FBasicProxy>> ProxysAry;

	FOnInventoryProxyStateChanged InventoryProxyStateChangedEvent;
	FDelegateHandle ItemDefineLoadedHandle;

	UPROPERTY(ReplicatedUsing = OnRep_ProxyContainer)
	FProxy_FASI_Container Proxy_FASI_Container;
};
