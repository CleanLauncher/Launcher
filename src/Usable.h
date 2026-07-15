#pragma once

#include <cstddef>
#include <memory>

#include "QObjectPtr.h"

class Usable;

class Usable {
    friend class UseLock;

   public:
    virtual ~Usable() {}

    std::size_t useCount() const { return m_useCount; }
    bool isInUse() const { return m_useCount > 0; }

   protected:
    virtual void decrementUses() { m_useCount--; }
    virtual void incrementUses() { m_useCount++; }

   private:
    std::size_t m_useCount = 0;
};

class UseLock {
   public:
    UseLock(Usable* usable) : m_usable(usable) { m_usable->incrementUses(); }
    ~UseLock() { m_usable->decrementUses(); }

   private:
    Usable* m_usable;
};
