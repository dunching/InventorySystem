#include "ItemProxy_Container.h"

#include "ItemProxy.h"

void FProxy_FASI::PostReplicatedAdd(
	const struct FProxy_FASI_Container& InArraySerializer
	)
{
	// 客户端新增条目时的扩展钩子（可用于 UI 绑定）。
	// ProxySPtr = MakeShared<>();
	
	InArraySerializer.OnAdd.Broadcast(ProxySPtr);
}

void FProxy_FASI::PostReplicatedChange(
	const struct FProxy_FASI_Container& InArraySerializer
	)
{
	// 客户端条目更新时的扩展钩子（可用于 UI 刷新）。
}

void FProxy_FASI::PreReplicatedRemove(
	const struct FProxy_FASI_Container& InArraySerializer
	)
{
	// 客户端删除条目前的扩展钩子（可用于资源回收）。
}

bool FProxy_FASI::NetSerialize(
	FArchive& Ar,
	class UPackageMap* Map,
	bool& bOutSuccess
	)
{
	if (!ProxySPtr.IsValid())
	{
		ProxySPtr = MakeShared<FBasicProxy>();
	}

	const bool Result = ProxySPtr->NetSerialize(Ar, Map, bOutSuccess);
	if (!Result)
	{
		return false;
	}

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
	if (ProxySPtr && Right.ProxySPtr)
	{
		return ProxySPtr->ProxyId == Right.ProxySPtr->ProxyId;
	}

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
			MarkArrayDirty();
			break;
		}
	}
}
