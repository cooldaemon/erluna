function fact (n)
  if n == 0 then
    return 1
  else
    return n * fact(n-1)
  end
end

lua_value = fact(5)
print(lua_value)

