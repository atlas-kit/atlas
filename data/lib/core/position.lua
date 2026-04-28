local mt = rawgetmetatable("Position")

function mt.__add(lhs, rhs)
	local stackpos = lhs.stackpos or rhs.stackpos
	return Position(lhs.x + (rhs.x or 0), lhs.y + (rhs.y or 0), lhs.z + (rhs.z or 0), stackpos)
end

function mt.__sub(lhs, rhs)
	local stackpos = lhs.stackpos or rhs.stackpos
	return Position(lhs.x - (rhs.x or 0), lhs.y - (rhs.y or 0), lhs.z - (rhs.z or 0), stackpos)
end

function mt.__concat(lhs, rhs) return tostring(lhs) .. tostring(rhs) end
function mt.__eq(lhs, rhs) return lhs.x == rhs.x and lhs.y == rhs.y and lhs.z == rhs.z end
function mt.__tostring(self) return string.format("Position(%d, %d, %d)", self.x, self.y, self.z) end

Position.directionOffset = {
	[DIRECTION_NORTH] = {x = 0, y = -1},
	[DIRECTION_EAST] = {x = 1, y = 0},
	[DIRECTION_SOUTH] = {x = 0, y = 1},
	[DIRECTION_WEST] = {x = -1, y = 0},
	[DIRECTION_SOUTHWEST] = {x = -1, y = 1},
	[DIRECTION_SOUTHEAST] = {x = 1, y = 1},
	[DIRECTION_NORTHWEST] = {x = -1, y = -1},
	[DIRECTION_NORTHEAST] = {x = 1, y = -1}
}

local abs, max = math.abs, math.max
function Position:getDistance(positionEx)
	local dx = abs(self.x - positionEx.x)
	local dy = abs(self.y - positionEx.y)
	local dz = abs(self.z - positionEx.z)
	return max(dx, dy, dz)
end

function Position:getDistanceX(positionOrX)
	local x = type(positionOrX) == "table" and positionOrX.x or positionOrX
	return abs(self.x - x)
end

function Position:getDistanceY(positionOrY)
	local y = type(positionOrY) == "table" and positionOrY.y or positionOrY
	return abs(self.y - y)
end

function Position:getDistanceZ(positionOrZ)
	local z = type(positionOrZ) == "table" and positionOrZ.z or positionOrZ
	return abs(self.z - z)
end

function Position:getNextPosition(direction, steps)
	local offset = Position.directionOffset[direction]
	if offset then
		steps = steps or 1
		self.x = self.x + offset.x * steps
		self.y = self.y + offset.y * steps
	end
end

function Position:moveUpstairs()
	local swap = function(lhs, rhs)
		lhs.x, rhs.x = rhs.x, lhs.x
		lhs.y, rhs.y = rhs.y, lhs.y
		lhs.z, rhs.z = rhs.z, lhs.z
	end

	self.z = self.z - 1

	local defaultPosition = self + Position.directionOffset[DIRECTION_SOUTH]
	local toTile = Tile(defaultPosition)
	if not toTile or not toTile:isWalkable() then
		local currentDirection
		for direction = DIRECTION_NORTH, DIRECTION_NORTHEAST do
			currentDirection = direction
			if currentDirection == DIRECTION_SOUTH then
				currentDirection = DIRECTION_WEST
			end

			local position = Position(self)
			position:getNextPosition(currentDirection)
			toTile = Tile(position)
			if toTile and toTile:isWalkable() then
				swap(self, position)
				return self
			end
		end
	end
	swap(self, defaultPosition)
	return self
end

function Position:isInRange(from, to)
	-- No matter what corner from and to is, we want to make
	-- life easier by calculating north-west and south-east
	local zone = {
		nW = {
			x = (from.x < to.x and from.x or to.x),
			y = (from.y < to.y and from.y or to.y),
			z = (from.z < to.z and from.z or to.z)
		},
		sE = {
			x = (to.x > from.x and to.x or from.x),
			y = (to.y > from.y and to.y or from.y),
			z = (to.z > from.z and to.z or from.z)
		}
	}

	if self.x >= zone.nW.x and self.x <= zone.sE.x
	and self.y >= zone.nW.y and self.y <= zone.sE.y
	and self.z >= zone.nW.z and self.z <= zone.sE.z then
		return true
	end
	return false
end

function Position:notifySummonAppear(summon)
	local spectators = Game.getSpectators(self)
	for _, spectator in ipairs(spectators) do
		if spectator:isMonster() and spectator ~= summon then
			spectator:addTarget(summon)
		end
	end
end

function Position:getDirectionTo(to)
    if self == to then
        return DIRECTION_NONE
    end

    local dir
    -- x_offset logic: from.x - to.x
    local x_offset = self.x - to.x
    
    if x_offset < 0 then
        dir = DIRECTION_EAST
        x_offset = math.abs(x_offset)
    else
        dir = DIRECTION_WEST
    end

    -- y_offset logic: from.y - to.y
    local y_offset = self.y - to.y
    
    if y_offset >= 0 then
        -- Logic for North (y decreases as you go North)
        if y_offset > x_offset then
            dir = DIRECTION_NORTH
        elseif y_offset == x_offset then
            if dir == DIRECTION_EAST then
                dir = DIRECTION_NORTHEAST
            else
                dir = DIRECTION_NORTHWEST
            end
        end
    else
        -- Logic for South (y increases as you go South)
        y_offset = math.abs(y_offset)
        if y_offset > x_offset then
            dir = DIRECTION_SOUTH
        elseif y_offset == x_offset then
            if dir == DIRECTION_EAST then
                dir = DIRECTION_SOUTHEAST
            else
                dir = DIRECTION_SOUTHWEST
            end
        end
    end

    return dir
end
