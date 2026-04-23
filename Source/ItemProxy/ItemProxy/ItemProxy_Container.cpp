#include "ItemProxy_Container.h"
#include "ItemProxy.h"

void FProxy_FASI::PostReplicatedAdd(
	const struct FProxy_FASI_Container& InArraySerializer
	)
{
	// 客户端收到“新增条目”增量包，通知外部系统（例如 UI、日志、调试面板）。
	InArraySerializer.OnAdd.Broadcast(ProxySPtr);
}

void FProxy_FASI::PostReplicatedChange(
	const struct FProxy_FASI_Container& InArraySerializer
	)
{
	// 客户端收到“条目变更”增量包，通知外部系统刷新该条目。
	InArraySerializer.OnChange.Broadcast(ProxySPtr);
}

void FProxy_FASI::PreReplicatedRemove(
	const struct FProxy_FASI_Container& InArraySerializer
	)
{
	// 客户端在条目被移除前收到回调，方便外部做清理（缓存/UI）。
	InArraySerializer.OnRemove.Broadcast(ProxySPtr);
}

bool FProxy_FASI::NetSerialize(
	FArchive& Ar,
	class UPackageMap* Map,
	bool& bOutSuccess
	)
{
	// 反序列化时先确保有可写入的 Proxy 对象。
	if (!ProxySPtr.IsValid())
	{
		ProxySPtr = MakeShared<FBasicProxy>();
	}

	// 实际字段序列化交由 FBasicProxy 处理。
	const bool Result = ProxySPtr->NetSerialize(Ar, Map, bOutSuccess);
	if (!Result)
	{
		return false;
	}

	// 预留扩展点：可在加载后根据 ItemTag 派生具体 Proxy 子类。
	if (Ar.IsLoading())
	{
	}

	bOutSuccess = true;
	return true;
}

bool FProxy_FASI::operator==(
	const FProxy_FASI& Right
	) const
{
	// 两边都有效时，按稳定 ID（ProxyId）比较身份。
	if (ProxySPtr && Right.ProxySPtr)
	{
		return ProxySPtr->ProxyId == Right.ProxySPtr->ProxyId;
	}

	// 否则退化为指针比较。
	return ProxySPtr == Right.ProxySPtr;
}

bool FProxy_FASI_Container::NetDeltaSerialize(
	FNetDeltaSerializeInfo& DeltaParms
	)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FItemType, FContainerType>(Items, DeltaParms, *this);
}

void FProxy_FASI_Container::AddItem(
	const TSharedPtr<FBasicProxy>& ProxySPtr
	)
{
	if (!ProxySPtr.IsValid())
	{
		return;
	}

	FProxy_FASI& NewItem = Items.AddDefaulted_GetRef();
	NewItem.ProxySPtr = ProxySPtr;

	// 标脏单条目，触发 FastArray 增量复制。
	MarkItemDirty(NewItem);
}

void FProxy_FASI_Container::UpdateItem(
	const TSharedPtr<FBasicProxy>& ProxySPtr
	)
{
	if (!ProxySPtr.IsValid())
	{
		return;
	}

	UpdateItem(ProxySPtr->ProxyId);
}

void FProxy_FASI_Container::UpdateItem(
	const FGuid& ProxyId
	)
{
	if (!ProxyId.IsValid())
	{
		return;
	}

	for (FProxy_FASI& Item : Items)
	{
		if (Item.ProxySPtr.IsValid() && Item.ProxySPtr->ProxyId == ProxyId)
		{
			// 标脏单条目，让客户端收到“变更”而不是全量重发。
			MarkItemDirty(Item);
			break;
		}
	}
}

void FProxy_FASI_Container::RemoveItem(
	const TSharedPtr<FBasicProxy>& ProxySPtr
	)
{
	if (!ProxySPtr.IsValid())
	{
		return;
	}

	for (int32 Index = 0; Index < Items.Num(); ++Index)
	{
		if (Items[Index].ProxySPtr.IsValid() && Items[Index].ProxySPtr->ProxyId == ProxySPtr->ProxyId)
		{
			Items.RemoveAt(Index);

			// 删除属于数组结构变化，需要标记整个数组脏。
			MarkArrayDirty();
			break;
		}
	}
}
