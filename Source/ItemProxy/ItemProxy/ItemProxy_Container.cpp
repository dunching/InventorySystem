#include "ItemProxy_Container.h"

#include "ItemProxy.h"

void FProxy_FASI::PostReplicatedAdd(
	const struct FProxy_FASI_Container& InArraySerializer
	)
{
	// 在这里 我们对本地的数据进行绑定
}

void FProxy_FASI::PostReplicatedChange(
	const struct FProxy_FASI_Container& InArraySerializer
	)
{
	// 在这里 我们对本地的数据进行绑定
}

void FProxy_FASI::PreReplicatedRemove(
	const struct FProxy_FASI_Container& InArraySerializer
	)
{
	// 在这里 我们对本地的数据进行绑定
}

bool FProxy_FASI::NetSerialize(
	FArchive& Ar,
	class UPackageMap* Map,
	bool& bOutSuccess
	)
{
	if (Ar.IsSaving())
	{
		return true;
	}
	else if (Ar.IsLoading())
	{
	}

	checkNoEntry();
	return false;
}

bool FProxy_FASI::operator==(
	const FProxy_FASI& Right
	) const
{
	if (ProxySPtr && Right.ProxySPtr)
	{
	}

	return true;
}

bool FProxy_FASI_Container::NetDeltaSerialize(
	FNetDeltaSerializeInfo& DeltaParms
	)
{
	const auto Result =
		FFastArraySerializer::FastArrayDeltaSerialize<FItemType, FContainerType>(Items, DeltaParms, *this);

	return Result;
}

void FProxy_FASI_Container::AddItem(
	const TSharedPtr<FBasicProxy>& ProxySPtr
	)
{
#if UE_EDITOR || UE_SERVER
#endif
}

void FProxy_FASI_Container::UpdateItem(
	const TSharedPtr<FBasicProxy>& ProxySPtr
	)
{
#if UE_EDITOR || UE_SERVER
#endif
}

void FProxy_FASI_Container::UpdateItem(
	const FGuid& Proxy_ID
	)
{
#if UE_EDITOR || UE_SERVER
#endif
}

void FProxy_FASI_Container::RemoveItem(
	const TSharedPtr<FBasicProxy>& ProxySPtr
	)
{
#if UE_EDITOR || UE_SERVER
#endif
}
