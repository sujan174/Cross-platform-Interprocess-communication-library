from cross_ipc import StoreDictPattern


dict_pattern = StoreDictPattern("test_dict", 1024, False)
dict_pattern.setup()


dict_pattern.store("hello", "world")

# Retrieve the value
value = dict_pattern.retrieve("hello")
print(f"Retrieved: {value}")


dict_pattern.close() 