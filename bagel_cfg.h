#pragma once

constexpr Bagel Params{
	.AggregateUpdates = true,
	.CallbackOnDestroy = true,
	.DynamicResize = false,
	.IdBagSize = 3000,
	.InitialEntities = 3000,
	.InitialPackedSize = 1000,
	.MaxComponents = 1000
};

//BAGEL_STORAGE(Position,PackedStorage)
