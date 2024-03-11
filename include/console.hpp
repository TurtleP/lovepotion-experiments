#pragma once

class Console
{
  public:
    enum Type
    {
        CTR,
        HAC,
        CAFE,
        ALL
    };

    static constexpr Type CURRENT = CTR;
};
