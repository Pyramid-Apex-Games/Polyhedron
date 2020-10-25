#pragma once

namespace recorder
{
    void stop();
    void capture(bool overlay = true);
    void cleanup();
}