def discretize(doe, factors):
    print("Discretizing the DOE")

    max_kb_idx = list(map(lambda x: len(x)-1,factors))

    for i in doe.index: #rows
        for j in range(len(list(doe.iloc[i]))): #cols
            idx = round(float(doe.iloc[i][j]) * max_kb_idx[j])
            elem = factors[j][idx]
            doe.iloc[i][j] = elem
