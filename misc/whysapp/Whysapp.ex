defmodule Whysapp do
  def connect() do
    opts = [:binary, active: false]
    host = 'challenges.fbctf.com'
    port = 4001
    :gen_tcp.connect(host, port, opts)
  end

  def pad(data, block_size) do
    to_add = block_size - rem(byte_size(data), block_size)
    length = String.length(data) + to_add
    padded = String.pad_trailing(data, length)
    data = :binary.copy(padded)
  end

  def send(sock, user_id, type, msg) do
    padded = Whysapp.pad("#{user_id}:#{type}:#{msg}", 16)
    encrypted = :crypto.block_encrypt(:aes_ecb, "yeetyeetyeetyeet", padded) |> :base64.encode
    :ok = :gen_tcp.send(sock, encrypted)
  end

  def recv(sock) do
    {:ok, msg} = :gen_tcp.recv(sock, 0)
    decoded = msg |> :base64.decode
    plaintext = :crypto.block_decrypt(:aes_ecb, "yeetyeetyeetyeet", decoded)
    msg = String.trim_trailing(plaintext, "\0")
    Whysapp.process(sock, msg)
  end

  def process(sock, msg) do
    user_id = :rand.uniform(1000000000)
    line = String.split(msg, ":")
    type = Enum.at(line, 0)
    msg = Enum.at(line, 1)
    case type do
      "msg" ->
        Whysapp.send(sock, user_id, 'msg', msg)
      "math" ->
        output = Kernel.elem(Code.eval_string(msg), 0)
        Whysapp.send(sock, user_id, 'math', output)
      "cats" ->
        Whysapp.send(sock, user_id, 'cats', 'cats')
      "ping" ->
        Whysapp.send(sock, user_id, 'ping', 'pong')
    end
  end
end
