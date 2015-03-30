luanovel.text._print = function (crt, str)
  local prtstr = luanovel.text._interpret(str)
  local ret = crt.name .. " \"" .. prtstr .. "\""
  luanovel.system.message(ret)
  return ret
end

luanovel.character.new = function ()
  local ret = { _img = { } }
  ret.say = function (self, str)
    return luanovel.text._print(self, str)
  end
  ret.addimage = function (self, index, filename)
    self._img[index] = luanovel.image.load(filename)
    return self._img[index]
  end
  ret.getimage = function (self, index)
    return self._img[index]
  end
  return ret
end

luanovel.system.message("Output mode is " .. luanovel.system.outputmode)
