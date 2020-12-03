#pragma once

#include "EventDetail.h"

using Event = detail::Event;

#include "GameEvent.h"
#include "StateEvent.h"
#include "EntityEvent.h"

#include "EventHandlerDetail.h"

#include "EntitySignalHandler.h"
#include "GameSignalHandler.h"
#include "StateSignalHandler.h"

void InitializeEventHandlers();