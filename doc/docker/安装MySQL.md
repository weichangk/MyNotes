### 拉取镜像
docker pull mysql
#docker pull mysql:8.0

### 启动临时容器
启动临时容器的目的主要是为后面步骤的安装挂载目录做准备工作，临时用容器后面步骤会删除
```
# 命令解释 docker run -p 宿主端口:容器端口 --name 容器名 -e root用户名属性=密码 -d 镜像名
docker run -p 3306:3306 --name mysql -e MYSQL_ROOT_PASSWORD=123456 -d mysql
```

### 创建挂载目录
MySQL相关配置以及数据库都将挂载到/home/mysql目录下；挂载的目的是为了数据持久化，防止容器删除后数据库的数据以及配置全部丢失。
```
# 创建挂载目录
mkdir /home/mysql

# 拷贝容器中的目录文件到宿主/home/mysql目录下
docker cp  mysql:/etc/mysql /home/mysql
# docker cp 容器名:容器内目录 宿主目录
```

### 删除临时容器
```
# 命令解释 docker rm -f 容器名或容器Id
docker rm -f mysql
```

### 配置MySQL
进入/home/mysql/mysql/conf.d/目录，新建my.cnf文件，将以下配置复制到my.cnf文件中
```
[mysqld]
# 限制MySQL内存占用
#performance_schema_max_table_instances=100
#table_definition_cache=100
#table_open_cache=64
#performance_schema=off

# 设置3306端口
port=3306
# 允许最大连接数
max_connections=1000
# 允许连接失败的次数。这是为了防止有人从该主机试图攻击数据库系统
max_connect_errors=100
# 服务端使用的字符集默认为UTF8
character-set-server=utf8mb4
# 创建新表时将使用的默认存储引擎
default-storage-engine=INNODB
#是否对sql语句大小写敏感，1表示不敏感
lower_case_table_names = 1
#MySQL连接闲置超过一定时间后(单位：秒)将会被强行关闭
#MySQL默认的wait_timeout  值为8个小时, interactive_timeout参数需要同时配置才能生效
interactive_timeout = 1800
wait_timeout = 1800
#Metadata Lock最大时长（秒）， 一般用于控制 alter操作的最大时长sine mysql5.6
#执行 DML操作时除了增加innodb事务锁外还增加Metadata Lock，其他alter（DDL）session将阻塞
lock_wait_timeout = 3600
#内部内存临时表的最大值。
#比如大数据量的group by ,order by时可能用到临时表，
#超过了这个值将写入磁盘，系统IO压力增大
tmp_table_size = 64M
max_heap_table_size = 64M
[mysql]
# 设置mysql客户端默认字符集
default-character-set=utf8mb4
[client]
# 设置mysql客户端连接服务端时默认使用的端口
port=3306
default-character-set=utf8mb4
```

### 创建容器
注意修改MYSQL_ROOT_PASSWORD的值，MYSQL_ROOT_PASSWORD设置的是MySQL数据库root的初始密码。
```
docker run \
-p 3306:3306 \
--name mysql8 \
--privileged=true \
--restart always \
-v /home/mysql/mysql:/etc/mysql \
-v /home/mysql/logs:/logs \
-v /home/mysql/data:/var/lib/mysql \
-v /etc/localtime:/etc/localtime \
-e MYSQL_ROOT_PASSWORD=123456 \
-d mysql
```

### 查看容器运行状态
```
docker ps -a
```

### 修改配置
进入MySQL容器修改MySQL配置

```
# 命令解释 docker exec -it 容器名或容器Id bash
docker exec -it mysql bash

mysql -u root -p #密码为上文创建容器设置的MYSQL_ROOT_PASSWORD的值。

```

设置root允许远程连接，依次执行以下命令
```
grant all PRIVILEGES on *.* to root@'%' WITH GRANT OPTION;

use mysql

update user set host='%' where user='root';
```

如果更新时提示一下错误:ERROR 1062 (23000): Duplicate entry '%-root' for key 'user.PRIMARY'，可通过SQL语句查询root是否已经被修改
```
select host,user from user where user='root';
```

设置可远程连接
```
grant all PRIVILEGES on *.* to root@'%' WITH GRANT OPTION;
ALTER user 'root'@'%' IDENTIFIED BY 'root用户的密码' PASSWORD EXPIRE NEVER;
ALTER user 'root'@'%' IDENTIFIED WITH mysql_native_password BY 'root用户的密码';
FLUSH PRIVILEGES;
```
