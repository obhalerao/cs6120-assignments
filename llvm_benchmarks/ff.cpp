#include <assert.h>
#include <cstdio>
#include <iostream>
#include <queue>
#include <vector>
#include "../lesson7/rtlib.hpp"

using namespace std;

const int max_num_cards = 40000;

int DFS(vector<vector<pair<int, int>>> &graph, int s, int t,
        vector<pair<int, int>> &prev, const vector<int> &rescap)
{
  vector<bool> visited = vector<bool>(graph.size(), false);
  vector<int> minima = vector<int>(graph.size(), 0);
  queue<int> q;
  q.push(s);
  minima[s] = max_num_cards;
  visited[0] = true;
  while (q.size() > 0)
  {
    int u = q.front();
    int f = minima[u];
    q.pop();
    for (pair<int, int> p : graph[u])
    {
      int v = p.first;
      int idx = p.second;
      if (!visited[v] && rescap[idx] > 0)
      {
        int new_f = min(f, rescap[idx]);
        q.push(v);
        minima[v] = new_f;
        visited[v] = true;
        prev[v] = {u, idx};
        if (v == t)
        {
          return minima[v];
        }
      }
    }
  }
  return 0;
}

int main()
{
  start_logging();
  int num_friends = 0;
  int num_vals = 0;
  int num_colors = 0;
  assert(scanf("%d %d %d\n", &num_friends, &num_vals, &num_colors) == 3);

  // initially, store the cards as lists of lists
  vector<vector<vector<short>>> card_counts;
  vector<vector<vector<int>>> card_ids;
  int global_card_id = 1; // card ids start at 1

  if (num_friends == -1 && num_vals == 200 && num_colors == 50)
  {

    num_friends = 20;
    num_colors = 40;
    num_vals = 50;

    // everybody gets one of every card
    card_counts = vector<vector<vector<short>>>(num_friends, vector<vector<short>>(num_vals, vector<short>(num_colors, 0)));
    card_ids = vector<vector<vector<int>>>(num_friends, vector<vector<int>>(num_vals, vector<int>(num_colors, 0)));

    for (int i = 0; i < num_friends; i++)
    {
      for (int j = 0; j < num_vals; j++)
      {
        for (int k = 0; k < num_colors; k++)
        {
          card_counts[i][j][k] = 1;
          card_ids[i][j][k] = global_card_id++;
        }
      }
    }
  }
  else
  {
    card_counts = vector<vector<vector<short>>>(num_friends, vector<vector<short>>(num_vals, vector<short>(num_colors, 0)));
    card_ids = vector<vector<vector<int>>>(num_friends, vector<vector<int>>(num_vals, vector<int>(num_colors, 0)));

    for (int i = 0; i < num_friends; i++)
    {

      int x;
      int c;

      while (cin.peek() != '\n' && cin.peek() != -1)
      {
        assert(scanf("%d %d", &x, &c) == 2);
        ++card_counts[i][x - 1][c - 1];
        if (card_counts[i][x - 1][c - 1] == 1)
        {
          card_ids[i][x - 1][c - 1] = global_card_id++;
        }
      }
      cin.get();
    }
  }
  // for (int i = 0; i < card_counts.size(); i++) {
  //   auto card_stack = card_counts[i];
  //   printf("next player's cards:\n");
  //   for (pair<int, int> card_info : card_stack) {
  //     int x = demap_x(card_info.first, num_vals, num_colors);
  //     int c = demap_c(card_info.first, num_vals, num_colors);
  //     printf("x = %d, c = %d, count = %d, global_id = %d\n", x, c,
  //            card_info.second, card_ids[i][card_info.first]);
  //   }
  // }

  // card with id i maps to a pair of vertices with ids 2i-1, 2i
  // source is 0
  // sink is 2 * global_card_id - 1
  int edge_id = 0; // edges zero-indexed
  int s = 0;
  int t = 2 * global_card_id - 1;
  vector<int> cap; // backwards edges initialized w cap 0

  vector<vector<pair<int, int>>> graph = vector<vector<pair<int, int>>>(
      2 * global_card_id, vector<pair<int, int>>());
  for (int i = 0; i < num_friends; i++)
  {
    for (int x = 0; x < num_vals; x++)
    {
      for (int c = 0; c < num_colors; c++)
      {

        int card_count = card_counts[i][x][c];
        if (card_count == 0)
          continue;

        int card_id = card_ids[i][x][c];

        cap.push_back(card_count);
        cap.push_back(0);
        graph[2 * card_id - 1].push_back({2 * card_id, edge_id++});
        graph[2 * card_id].push_back({2 * card_id - 1, edge_id++});

        if (x == 0)
        {
          cap.push_back(card_count);
          cap.push_back(0);
          graph[s].push_back({2 * card_id - 1, edge_id++});
          graph[2 * card_id - 1].push_back({s, edge_id++});
        }
        else if (x == num_vals - 1)
        { // m at least 2
          cap.push_back(card_count);
          cap.push_back(0);
          graph[2 * card_id].push_back({t, edge_id++});
          graph[t].push_back({2 * card_id, edge_id++});
        }

        // same player plays x + 1, c
        if (x + 1 < num_vals && card_ids[i][x + 1][c])
        {
          int other_id = card_ids[i][x + 1][c];
          cap.push_back(max_num_cards);
          cap.push_back(0);
          graph[2 * card_id].push_back({2 * other_id - 1, edge_id++});
          graph[2 * other_id - 1].push_back({2 * card_id, edge_id++});
        }

        // same player plays x, c'
        for (int color = 0; color < num_colors; color++)
        {
          if (color == c)
            continue;
          if (x + 1 < num_vals && card_ids[i][x][color])
          {
            int other_id = card_ids[i][x][color];
            cap.push_back(max_num_cards);
            cap.push_back(0);
            graph[2 * card_id].push_back({2 * other_id - 1, edge_id++});
            graph[2 * other_id - 1].push_back({2 * card_id, edge_id++});
          }
        }

        if (num_friends > 1)
        {
          // next player plays x + 1, c
          if (x + 1 < num_vals && card_ids[(i + 1) % num_friends][x + 1][c])
          {
            int other_id = card_ids[(i + 1) % num_friends][x + 1][c];
            cap.push_back(max_num_cards);
            cap.push_back(0);
            graph[2 * card_id].push_back({2 * other_id - 1, edge_id++});
            graph[2 * other_id - 1].push_back({2 * card_id, edge_id++});
          }

          // next player plays (x, c')
          for (int color = 0; color < num_colors; color++)
          {
            if (color == c)
              continue;
            if (card_ids[(i + 1) % num_friends][x][color])
            {
              int other_id = card_ids[(i + 1) % num_friends][x][color];
              cap.push_back(max_num_cards);
              cap.push_back(0);
              graph[2 * card_id].push_back({2 * other_id - 1, edge_id++});
              graph[2 * other_id - 1].push_back({2 * card_id, edge_id++});
            }
          }
        }
      }
    }
  }

  // for (int i = 0; i < graph.size(); i++) {
  //   vector<pair<int, int>> neighbors = graph[i];
  //   printf("%d: ", i);
  //   for (pair<int, int> pair : neighbors) {
  //     printf("(%d, %d), ", pair.first, cap[pair.second]);
  //   }
  //   printf("\n");
  // }

  // for (int i : cap) {
  //   printf("%d ", i);
  // }
  // printf("\n");

  // printf("%lu\n", graph.size());
  // printf("%lu\n", cap.size());

  int ans = 0;
  auto prev = vector<pair<int, int>>(graph.size(), {-1, -1});
  vector<int> rescap = vector<int>(cap);
  int f = DFS(graph, s, t, prev, rescap);
  int count = 0;

  while (f)
  {
    // if (count % 100 == 0)
    //   fprintf(stderr, "%d: %d\n", count++, f);
    // else
    //   count++;
    int v = t;
    int sub_count = 1;
    while (v != s)
    {
      auto p = prev[v];
      int u = p.first;
      int idx = p.second;
      int rev_idx = (cap[idx] == 0) ? idx - 1 : idx + 1;
      rescap[idx] -= f;
      rescap[rev_idx] += f;
      v = u;
      sub_count++;
    }
    // fprintf(stderr, "%d\n", sub_count);
    ans += f;
    f = DFS(graph, s, t, prev, rescap);
  }
  printf("%d\n", ans);

  end_logging();

  // for (int i = 0; i < num_friends; i++)
  // {
  //   for (int x = 0; x < num_vals; x++)
  //   {
  //     for (int c = 0; c < num_colors; c++)
  //     {

  //       int card_count = card_counts[i][x][c];
  //       if (card_count == 0)
  //         continue;
  //       int card_id = card_ids[i][x][c];
  //       int total_flow = 0;
  //       for (auto edge_info : graph[card_id]){
  //         int v = edge_info.first;
  //         int idx = edge_info.second;
  //         if (cap[idx] != 0 && rescap[idx] < cap[idx]){
  //           total_flow += cap[idx] - rescap[idx];
  //         }
  //       }
  //       if (total_flow > 0){
  //       printf("%d\n", total_flow);
  //       }

  //     }
  //   }
  // }
}