#pragma once

#include "market_data.pb.h"

class IOrderBookListener {
public:
    virtual void OnOrderBookUpdate(int instrument_id, const marketdata::OrderbookIncrementalUpdate& update) = 0;
    virtual ~IOrderBookListener() = default;
};