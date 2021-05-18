
# a = 'abc'
# b = "abc"
# print(a) # 输出a的值
# print(type(a)) # 输出a的类型

# print(b) # 输出b的值
# print(type(b)) # 输出b的类型

# a = """1234
# 123"""

# a = "我说:\"你在干什么\""
# print(a)

# a = "123"
# b = "456"
# c = a + b
# print(c)


# a = "123"
# a = a * 2
# print(a)


# a = "HelloWorld!" # a[n:m]的意思表示截取a字符串里面从n位置开始到m位置的所有字符
# print(a[:a.__len__()]) # 输出整个字符串
# print(a[:-1]) # 这边后面填写-1也表示输出整个字符串
# print(a[:6]) # 表示输出从0开始到第六个字符的字符串
# print(a[1:3]) #这个表示输出1-3的字符

# # 字符串替换
# a = "He-llo-W-o-rld"
# a = a.replace('-','') # 把-替换成空，相当于把-去掉
# print(a)

# # 字符串分割
# a = "He-llo-W-o-rld"
# b = a.split('-') # 通过搜索-，对字符串进行分割
# print(b)
# print(type(b))

# # 字符串查找
# a = "Helloword!"
# b = a.find('ll') # 左边开始查找
# c = a.rfind('ll')   # 右边开始查找
# d = a.index('ll')# 左边开始查找
# e = a.rindex('ll') # 右边开始查找
# print(b)
# print(c)
# print(d)
# print(e)

# # 计算字符串出现次数
# a = "Helloword!"
# b = a.count('ll')   # ll出现的次数
# c = a.count('l')    # l出现的次数
# d = a.count('o')    # o出现的次数
# print(b)
# print(c)
# print(d)

# 计算字符串大小写
a = "hello word!"
print(a.capitalize())
print(a.title())
print(a.lower())
print(a.upper())