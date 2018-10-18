/// Make code const correct
/// Remove all these "Init" and "Destroy" functions save for things like swapchain cleanup as that must happen mid-program
/// Replace all those printf calls with proper error logging
/// Make a better error handling system (logging instead of exceptions ty)
/// Organize program files into folders (at least into src/header)
/// Make VertexBuffers more efficient (no memcpy pls)
/// add functionality for texture loading and rendering
/// add depth buffering