#pragma once

#include "ITheme.h"

class MinimalTheme : public ITheme {
   public:
    virtual ~MinimalTheme() {}

    QString qtTheme() override;
    QString appStyleSheet() override;
};
