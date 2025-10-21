# Sistema de Autenticación Distribuido

## Arquitectura

El sistema está compuesto por tres nodos principales:

1. **Nodo de Autenticación** (puerto 5001)
2. **Nodo Proxy/Intermedio** (puerto 5002)
3. **Cliente**



### Escenario 1: Cliente → Nodo de Autenticación (Directo)

```
1. Cliente envía AuthRequest (usuario, contraseña) → Nodo Auth
2. Nodo Auth valida credenciales consultando el filesystem
   - Lee archivo de usuarios del filesystem
   - Compara hash de contraseña
   - Verifica intentos fallidos
   - Obtiene rol del usuario
3. Nodo Auth responde:
   - Si es exitoso: AuthResponse (token, rol) → Cliente
   - Si falla: AuthError (código de error) → Cliente
4. Cliente recibe respuesta y muestra UI según el rol
```

### Escenario 2: Cliente → Nodo Intermedio → Nodo de Autenticación

```
1. Cliente envía AuthRequest (usuario, contraseña) → Nodo Proxy
2. Nodo Proxy reenvía AuthRequest → Nodo Auth
3. Nodo Auth valida credenciales consultando el filesystem
   - Lee archivo de usuarios del filesystem
   - Compara hash de contraseña
   - Verifica intentos fallidos
   - Obtiene rol del usuario
4. Nodo Auth responde al Proxy:
   - Si es exitoso: AuthResponse (token, rol) → Nodo Proxy
   - Si falla: AuthError (código de error) → Nodo Proxy
5. Nodo Auth notifica al Proxy: TokenRegister (token, username, rol)
6. Nodo Proxy reenvía respuesta → Cliente
7. Cliente recibe respuesta y muestra UI según el rol
```

### Validación de Sesión (Posterior al Login)

```
1. Cliente envía SessionValidate (username, token) → Nodo Proxy
2. Nodo Proxy valida token:
   - Busca username en su registro de tokens
   - Compara token recibido con token almacenado
3. Nodo Proxy responde:
   - Si es válido: SessionOk (rol) → Cliente
   - Si es inválido: SessionError (código de error) → Cliente
```

## Mensajes del Protocolo

### Autenticación
- **MSG_AUTHENTICATION (1)**: Cliente solicita autenticación
- **MSG_AUTH_RESPONSE (2)**: Autenticación exitosa (34 bytes: id + token[32] + role)
- **MSG_AUTH_ERROR (3)**: Error de autenticación (2 bytes: id + error_code)

### Validación de Sesión
- **MSG_TOKEN_REGISTER (4) - TokenNotif**: Auth registra token en Proxy (50 bytes: id + token[32] + username[16] + role)
- **MSG_SESSION_VALIDATE (5)**: Cliente valida sesión (49 bytes: id + username[16] + token[32])
- **MSG_SESSION_OK (6)**: Sesión válida (2 bytes: id + role)
- **MSG_SESSION_ERROR (7)**: Sesión inválida (2 bytes: id + error_code)

## Códigos de Error

- **1**: Credenciales incorrectas
- **2**: Exceso de intentos de autenticación
- **3**: Token inválido o expirado

## Compilación

```bash
# Compilar servidor de autenticación
g++ -std=c++17 -I./sockets sockets/authMain.cpp -o authServer -pthread

# Compilar servidor proxy
g++ -std=c++17 -I./sockets sockets/proxyMain.cpp -o proxyServer -pthread

# Compilar cliente
g++ -std=c++17 -I./sockets sockets/clientMain.cpp -o client -pthread
```

## Ejecución

**Importante**: Ejecutar en este orden en terminales separadas:

```bash
# Terminal 1: Iniciar nodo de autenticación
./authServer

# Terminal 2: Iniciar nodo proxy
./proxyServer

# Terminal 3: Ejecutar cliente
./client
```

Para detener los servidores, escribe `#` y presiona Enter.

## Estado de Implementación

### ✅ Integrado y Funcional

#### 1. Nodo de Autenticación (NodeAuth)
- **Integración con Filesystem**: ✅ COMPLETA
  - El método `validateCredentials()` consulta el filesystem para verificar credenciales
  - Lee archivos de usuarios almacenados en el filesystem
  - Compara hashes de contraseñas
  - Verifica intentos fallidos de autenticación
  - Obtiene y retorna el rol real del usuario (admin, analista, auditor, etc.)
- **Generación de Tokens**: ✅ COMPLETA
  - Genera tokens únicos de 32 bytes para sesiones autenticadas
- **Notificación al Proxy**: ✅ COMPLETA
  - Envía mensaje `TokenRegister` al proxy con token, username y rol

#### 2. Protocolo de Comunicación
- **Estructura de Datagramas**: ✅ VALIDADA
  - Todos los mensajes siguen las estructuras definidas en el trabajo anterior
  - Tamaños fijos y campos correctamente alineados
  - Tipos de mensaje: AuthRequest, AuthResponse, AuthError, TokenRegister, SessionValidate, SessionOk, SessionError
- **Validación de Permisos y Roles**: ✅ IMPLEMENTADA
  - El sistema identifica y propaga roles de usuario (admin, analista, auditor)
  - Los roles se verifican en el nodo de autenticación consultando el filesystem

#### 3. Cliente UI
- **Interfaz Diferenciada por Rol**: ✅ IMPLEMENTADA
  - El cliente recibe el rol del usuario en la respuesta de autenticación
  - Muestra UI con accesos a ventanas distintas según el rol:
    - **Admin**: Acceso completo a todas las funcionalidades
    - **Analista**: Acceso a consultas y análisis de datos
    - **Auditor**: Acceso de solo lectura para auditorías
  - La UI se adapta dinámicamente según los permisos del rol

### 🔄 Pendiente de Integración

#### Nodo Proxy (NodeProxy)
- **Persistencia de Tokens en Filesystem**: ⚠️ PENDIENTE
  - **Estado actual**: Los tokens se guardan en memoria usando `std::map<username, TokenData>`
  - **Integración requerida**:
    - Modificar `onTokenRegister()` para guardar tokens en archivo del filesystem
    - Modificar `isTokenValid()` para leer tokens desde archivo del filesystem
    - Formato sugerido: archivo con líneas `username:token:role`
  - **Impacto**: Esta es la única integración faltante para completar la persistencia del sistema

**Cambio mínimo requerido en NodeProxy**: 
- Reemplazar operaciones de `std::map<std::string, TokenData> validTokens` por operaciones de lectura/escritura en archivos del filesystem
- La clave de búsqueda es el **username**, el valor almacenado es el **token** y el **role**

## Verificación de Requisitos del Trabajo

### 1. Login de Usuarios ✅

#### Primera Prueba: Cliente → Nodo Servidor de Autenticación
- ✅ **Implementado y funcional**
- El cliente se conecta directamente al nodo de autenticación (puerto 5001)
- Envía credenciales y recibe respuesta con token y rol
- El nodo de autenticación valida contra el filesystem

#### Segunda Prueba: Cliente → Nodo Intermedio → Nodo Servidor de Autenticación
- ✅ **Implementado y funcional**
- El cliente se conecta al nodo proxy (puerto 5002)
- El proxy reenvía la petición al nodo de autenticación (puerto 5001)
- La respuesta fluye de regreso: Auth → Proxy → Cliente
- El nodo de autenticación notifica al proxy sobre tokens válidos

### 2. Validaciones ✅

#### Verificación de Estructuras (Datagramas)
- ✅ **Validado completamente**
- Todas las estructuras de mensajes definidas en el trabajo anterior están implementadas
- Tamaños de datagrama correctos y verificados:
  - `AuthRequest`: 49 bytes (id + username[16] + password[32])
  - `AuthResponse`: 34 bytes (id + token[32] + role)
  - `AuthError`: 2 bytes (id + error_code)
  - `TokenRegister`: 50 bytes (id + token[32] + username[16] + role)
  - `SessionValidate`: 49 bytes (id + username[16] + token[32])
  - `SessionOk`: 2 bytes (id + role)
  - `SessionError`: 2 bytes (id + error_code)

#### Verificación de Permisos y Roles de Usuario
- ✅ **Implementado y funcional**
- El sistema identifica roles: admin, analista, auditor
- Los roles se obtienen del filesystem durante la autenticación
- Los roles se propagan correctamente a través de los nodos
- El cliente UI adapta la interfaz según el rol recibido

## Próximos Pasos (Fuera del Scope Actual)

Después de una sesión válida, el sistema podría extenderse para:
1. **Solicitar datos de sensores**: Enviar peticiones al proxy
2. **Proxy balancea carga**: Distribuir peticiones entre nodos de storage
3. **Recibir y mostrar datos**: Presentar información al usuario según permisos

Para esto se necesitarían nuevos tipos de mensajes (ej. `MSG_DATA_REQUEST`, `MSG_DATA_RESPONSE`) y la implementación de los nodos de storage.
