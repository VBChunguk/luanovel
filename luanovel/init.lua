luanovel.text._interpret = function (str)
  local ret = str
  ret = string.gsub(ret, "\\{", "\x04")
  ret = string.gsub(ret, "\\}", "\x05")

  ret = string.gsub(ret, "{(.-)}", function (s)
    return load("return (" .. s .. ")")()
  end)

  ret = string.gsub(ret, "\x04", "{")
  ret = string.gsub(ret, "\x05", "}")
  return ret
end

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


luanovel.status.text = { str="", position=0, timer=0 }
luanovel.internal.text = { timer_constant=2 }

luanovel.internal.text_on_step = function ()
  local txt = luanovel.status.text
  if not txt then
    return
  end
  txt.timer = txt.timer - 1
  if txt.timer <= 0 then
    txt.timer = luanovel.internal.text.timer_constant
    txt.position = txt.position - 1
    if txt.position < 0 then
      txt.position = 0
    end
  end
end

luanovel.text.setstring = function (str)
  local txt = luanovel.status.text
  txt.str = str
  txt.position = utf8.len(str)
  txt.timer = luanovel.internal.text.timer_constant
end

luanovel.text.draw = function (cr, x, y, font)
  local rdr = luanovel.rendering
  local txt = luanovel.status.text
  local off = #txt.str
  if off <= 0 then
    return
  end

  if txt.position > 0 then
    off = utf8.offset(txt.str, -txt.position)-1
  end
  rdr.drawtext(cr, string.sub(txt.str, 1, off), x, y, font)
end

luanovel.text.advance = function ()
  local txt = luanovel.status.text
  if txt.position == 0 then
    
  else
    txt.position = 0
  end
end

luanovel.internal.on_step = function (phase)
  luanovel.internal.text_on_step()
end