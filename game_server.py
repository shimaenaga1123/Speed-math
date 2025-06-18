#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import socket
import threading
import time
import json
from datetime import datetime

class GameServer:
    def __init__(self, host='103.244.118.110', port=13579):
        self.host = host
        self.port = port
        self.leaderboard = []  # [(score, timestamp, player_id), ...]
        self.player_counter = 0
        self.server_socket = None
        
    def start_server(self):
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        
        try:
            self.server_socket.bind((self.host, self.port))
            self.server_socket.listen(10)
            print(f"🚀 게임 서버가 {self.host}:{self.port}에서 시작되었습니다!")
            print("=" * 60)
            print("📊 실시간 순위 시스템 가동 중...")
            print("🎮 클라이언트 연결을 기다리는 중...")
            print("=" * 60)
            
            while True:
                try:
                    client_socket, address = self.server_socket.accept()
                    self.player_counter += 1
                    player_id = f"Player_{self.player_counter}"
                    
                    print(f"🎯 새로운 플레이어 연결: {player_id} ({address[0]}:{address[1]})")
                    
                    # 클라이언트 처리를 위한 스레드 시작
                    client_thread = threading.Thread(
                        target=self.handle_client,
                        args=(client_socket, player_id, address)
                    )
                    client_thread.daemon = True
                    client_thread.start()
                    
                except Exception as e:
                    print(f"❌ 클라이언트 수락 오류: {e}")
                    
        except Exception as e:
            print(f"❌ 서버 시작 오류: {e}")
        finally:
            if self.server_socket:
                self.server_socket.close()
    
    def handle_client(self, client_socket, player_id, address):
        try:
            while True:
                try:
                    # 클라이언트로부터 데이터 받기
                    data = client_socket.recv(1024).decode('utf-8')
                    if not data:
                        break
                    
                    print(f"📨 {player_id}로부터 받은 데이터: {data}")
                    
                    # 점수 데이터 처리
                    if data.startswith("SCORE:"):
                        score_str = data[6:]  # "SCORE:" 제거
                        try:
                            score = float(score_str)
                            timestamp = datetime.now()
                            
                            # 순위표에 추가
                            self.add_score(score, timestamp, player_id)
                            
                            # 현재 순위 계산
                            rank = self.get_player_rank(player_id, score, timestamp)
                            
                            # 순위 정보 전송
                            rank_info = f"#{rank} (점수: {score:.2f}ms, 총 {len(self.leaderboard)}명 중)"
                            client_socket.send(rank_info.encode('utf-8'))
                            
                            print(f"🏆 {player_id} - 점수: {score:.2f}ms, 순위: #{rank}")
                            self.print_leaderboard()
                            
                        except ValueError:
                            print(f"❌ 잘못된 점수 형식: {score_str}")
                            client_socket.send("ERROR: Invalid score format".encode('utf-8'))
                    
                except socket.timeout:
                    continue
                except Exception as e:
                    print(f"❌ {player_id} 데이터 처리 오류: {e}")
                    break
                    
        except Exception as e:
            print(f"❌ {player_id} 연결 오류: {e}")
        finally:
            client_socket.close()
            print(f"🔌 {player_id} 연결 해제 ({address[0]}:{address[1]})")
    
    def add_score(self, score, timestamp, player_id):
        """점수를 순위표에 추가 (낮은 점수가 더 좋음)"""
        self.leaderboard.append((score, timestamp, player_id))
        # 점수 순으로 정렬 (낮은 점수가 1등)
        self.leaderboard.sort(key=lambda x: x[0])
        
        # 상위 100명만 유지
        if len(self.leaderboard) > 100:
            self.leaderboard = self.leaderboard[:100]
    
    def get_player_rank(self, player_id, score, timestamp):
        """특정 플레이어의 현재 순위 반환"""
        for i, (s, t, pid) in enumerate(self.leaderboard):
            if pid == player_id and s == score and t == timestamp:
                return i + 1
        return len(self.leaderboard)
    
    def print_leaderboard(self):
        """현재 순위표 출력"""
        print("\n" + "=" * 60)
        print("🏆 현재 순위표 (TOP 10)")
        print("=" * 60)
        print(f"{'순위':<4} {'플레이어':<12} {'점수(ms)':<10} {'시간':<20}")
        print("-" * 60)
        
        for i, (score, timestamp, player_id) in enumerate(self.leaderboard[:10]):
            time_str = timestamp.strftime("%H:%M:%S")
            print(f"{i+1:<4} {player_id:<12} {score:<10.2f} {time_str:<20}")
        
        if len(self.leaderboard) > 10:
            print(f"... 그 외 {len(self.leaderboard) - 10}명")
        
        print("=" * 60 + "\n")
    
    def stop_server(self):
        """서버 중지"""
        if self.server_socket:
            self.server_socket.close()
            print("🛑 서버가 중지되었습니다.")

def main():
    print("🎮 256비트 큰 수 덧셈 게임 서버")
    print("=" * 40)
    
    server = GameServer()
    
    try:
        server.start_server()
    except KeyboardInterrupt:
        print("\n🛑 서버 중지 중...")
        server.stop_server()
    except Exception as e:
        print(f"❌ 서버 오류: {e}")
        server.stop_server()

if __name__ == "__main__":
    main()