local ok, err = pcall(function()
  print("/: ", fs.direntries("."))
  print("fs.mkdir(\"/nstsrc\")")
  fs.mkdir("/nstsrc")
  print("/: ", fs.direntries("."))
  print("xio.writefile(\"/nstsrc/helloworld.txt\", \"Hello, World!\n\")")
  xio.writefile("/nstsrc/helloworld.txt", "w", "Hello, World!\n")
  print("/: ", fs.direntries("."))
  print("/nstsrc: ", fs.direntries("/nstsrc"))

  print("Starting the sub-builder.")
  build("/nstsrc", "/nstwkplc", "/dest/nstdest", function()
    print("The sub-builder started.")
    fs.copy("/src/helloworld.txt", "/dest/helloworld.txt")
    xio.writefile("/dest/anotherfile.txt", ")@$&@#$&@#($&@#(")
  end)
end)

if not ok then
  print(err)
end
