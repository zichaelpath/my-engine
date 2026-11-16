-- patrol.lua -- a guard who walks his beat.
--
-- This NPC's ENTIRE behaviour lives in this file. The engine has
-- no patrol code in it at all: no C++ knows what a beat is, what
-- the range is, or when to turn around. Attach this script to an
-- entity (its type, or the entity itself in a scene) and it walks.
--
-- Edit SPEED or RANGE, save, and -- with the game running -- the
-- new beat takes over within half a second (hot reload, L71).
-- The guard re-captures his home wherever he is standing when
-- the script (re)starts: home is where the shift began.

local SPEED = 60   -- units per second
local RANGE = 120  -- how far from home, each way

local M = {}

function M.onUpdate(self, dt)
    -- First frame of a shift: remember home, face right.
    if self.dir == nil then
        local x, _ = entity.getPosition(self)
        self.homeX = x
        self.dir = 1
    end

    local x, y = entity.getPosition(self)
    local nextX = x + self.dir * SPEED * dt

    -- The ends of the beat: stop exactly on the bound and turn.
    if nextX > self.homeX + RANGE then
        nextX = self.homeX + RANGE
        self.dir = -1
    elseif nextX < self.homeX - RANGE then
        nextX = self.homeX - RANGE
        self.dir = 1
    end

    entity.setPosition(self, nextX, y)
end

return M
