describe("Network.TCPSocket",{
	// ti.network.HTTP objects are covered in the http_server unit test.

	before_all: function()
	{
		value_of(Titanium.Network).should_not_be_null();
		value_of(Titanium.Network.createTCPSocket).should_be_function();
		value_of(Titanium.Network.TCPSocket).should_not_be_null();

		// Launch testing server on port 8080
		// Launch test http server
		this.testServer = Titanium.Process.createProcess(
		{
			args: [
				'python', Titanium.API.application.resourcesPath + "/testserver.py"
			],
		});
		this.testServer.launch();
		
		// Create a test socket client
		this.socket = Titanium.Network.createTCPSocket("127.0.0.1", 8080);
	},
	
	after_all: function()
	{
		this.socket.close();
		this.testServer.kill();
	},

	// test the network object and properties.
	test_TCPSocket_object: function()
	{
		value_of(this.socket).should_be_object();	
		value_of(this.socket.close).should_be_function();
		value_of(this.socket.connect).should_be_function();
		value_of(this.socket.isClosed).should_be_function();
		value_of(this.socket.onRead).should_be_function();
		value_of(this.socket.onReadComplete).should_be_function();
		value_of(this.socket.onTimeout).should_be_function();
		value_of(this.socket.onWrite).should_be_function();
		value_of(this.socket.write).should_be_function();
	},
	
	test_isClosed: function()
	{
		value_of(this.socket.isClosed).should_be_function();
		value_of(this.socket.isClosed()).should_be_true();
	},
	
	test_connect_as_async: function(test)
	{
		var timer;

		this.socket.on("connect", function()
		{
			try
			{
				clearTimeout(timer);
				value_of(this.socket.isClosed()).should_be_false();
				test.passed();
			catch (e)
			{
				test.failed(e);
			}
		});

		this.socket.connect();
		timer = setTimeout(function()
		{
			test.failed("Test timed out");
		}, 2000);
	},

	test_read_write_as_async: function(test)
	{
		var timer;
		var message = "hello, can anyone hear me?";

		this.socket.on("connect", function()
		{
			// Send test server a message.
			this.socket.write(message);
		});

		this.socket.on("data", function(data)
		{
			clearTimeout(timer);

			try
			{
				// Test server should echo the message back.
				value_of(data).should_be(message);
				test.passed();
			}
			catch (e)
			{
				test.failed(e);
			}
		});

		this.socket.connect();
		timer = setTimeout(function()
		{
			test.failed("Timed out waiting for data");
		}, 2000);
	}
});
